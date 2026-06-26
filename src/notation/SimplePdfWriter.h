#pragma once

#include <JuceHeader.h>
#include <vector>

class SimplePdfWriter
{
public:
    static bool writeImagePages(const juce::File& destination,
                                const juce::Array<juce::Image>& pages,
                                juce::String& error)
    {
        error.clear();

        if (pages.isEmpty())
        {
            error = "No pages were provided for PDF export.";
            return false;
        }

        if (!ensureParentDirectoryExists(destination, error))
            return false;

        struct EncodedPage
        {
            int width = 0;
            int height = 0;
            juce::MemoryBlock jpegData;
        };

        std::vector<EncodedPage> encodedPages;
        encodedPages.reserve(static_cast<size_t>(pages.size()));

        for (const auto& page : pages)
        {
            if (!page.isValid() || page.getWidth() <= 0 || page.getHeight() <= 0)
            {
                error = "Invalid page image encountered during PDF export.";
                return false;
            }

            juce::JPEGImageFormat jpegFormat;
            jpegFormat.setQuality(0.92f);
            juce::MemoryOutputStream jpegStream;
            if (!jpegFormat.writeImageToStream(page, jpegStream))
            {
                error = "Failed to encode a PDF page as JPEG.";
                return false;
            }

            EncodedPage encoded;
            encoded.width = page.getWidth();
            encoded.height = page.getHeight();
            encoded.jpegData = jpegStream.getMemoryBlock();
            encodedPages.push_back(std::move(encoded));
        }

        juce::MemoryOutputStream pdfStream;

        const int pageCount = static_cast<int>(encodedPages.size());
        const int objectCount = 2 + pageCount * 3;
        std::vector<int64_t> objectOffsets(static_cast<size_t>(objectCount + 1), 0);

        pdfStream.writeString("%PDF-1.4\n%\xFF\xFF\xFF\xFF\n");

        int nextObject = 1;
        auto beginObject = [&](int objectNumber)
        {
            objectOffsets[(size_t) objectNumber] = pdfStream.getDataSize();
            pdfStream.writeString(juce::String(objectNumber) + " 0 obj\n");
        };
        auto endObject = [&]()
        {
            pdfStream.writeString("\nendobj\n");
        };

        beginObject(nextObject++); // Catalog
        pdfStream.writeString("<< /Type /Catalog /Pages 2 0 R >>");
        endObject();

        beginObject(nextObject++); // Pages
        pdfStream.writeString("<< /Type /Pages /Count " + juce::String(pageCount) + " /Kids [");
        for (int i = 0; i < pageCount; ++i)
        {
            const int pageObject = 3 + i * 3;
            pdfStream.writeString(juce::String(pageObject) + " 0 R ");
        }
        pdfStream.writeString("] >>");
        endObject();

        for (int i = 0; i < pageCount; ++i)
        {
            const int pageObject = 3 + i * 3;
            const int imageObject = pageObject + 1;
            const int contentObject = pageObject + 2;
            const auto& page = encodedPages[(size_t) i];
            const juce::String imageName = "Im" + juce::String(i + 1);

            beginObject(pageObject);
            pdfStream.writeString("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 "
                                  + juce::String(page.width) + " " + juce::String(page.height)
                                  + "] /Resources << /XObject << /" + imageName + " "
                                  + juce::String(imageObject) + " 0 R >> >> /Contents "
                                  + juce::String(contentObject) + " 0 R >>");
            endObject();

            beginObject(imageObject);
            pdfStream.writeString("<< /Type /XObject /Subtype /Image /Width "
                                  + juce::String(page.width) + " /Height " + juce::String(page.height)
                                  + " /ColorSpace /DeviceRGB /BitsPerComponent 8 /Filter /DCTDecode /Length "
                                  + juce::String((int) page.jpegData.getSize()) + " >>\nstream\n");
            pdfStream.write(page.jpegData.getData(), page.jpegData.getSize());
            pdfStream.writeString("\nendstream");
            endObject();

            const juce::String content = "q\n"
                                         + juce::String(page.width) + " 0 0 " + juce::String(page.height) + " 0 0 cm\n"
                                         + "/" + imageName + " Do\n"
                                         + "Q\n";
            beginObject(contentObject);
            pdfStream.writeString("<< /Length " + juce::String(content.getNumBytesAsUTF8()) + " >>\nstream\n");
            pdfStream.writeString(content);
            pdfStream.writeString("endstream");
            endObject();
        }

        const int64_t xrefOffset = pdfStream.getDataSize();
        pdfStream.writeString("xref\n0 " + juce::String(objectCount + 1) + "\n");
        pdfStream.writeString("0000000000 65535 f \n");
        for (int object = 1; object <= objectCount; ++object)
        {
            const auto offset = objectOffsets[(size_t) object];
            pdfStream.writeString(juce::String::formatted("%010lld 00000 n \n", static_cast<long long>(offset)));
        }

        pdfStream.writeString("trailer\n<< /Size " + juce::String(objectCount + 1) + " /Root 1 0 R >>\n");
        pdfStream.writeString("startxref\n" + juce::String(xrefOffset) + "\n%%EOF\n");

        if (!destination.replaceWithData(pdfStream.getData(), pdfStream.getDataSize()))
        {
            error = "Failed to write PDF file to disk.";
            return false;
        }

        if (!hasPdfHeader(destination))
        {
            error = "Generated PDF did not pass header validation.";
            return false;
        }

        return true;
    }

private:
    static bool ensureParentDirectoryExists(const juce::File& file, juce::String& error)
    {
        const auto parent = file.getParentDirectory();
        if (parent == juce::File())
        {
            error = "Invalid destination path.";
            return false;
        }
        if (parent.exists())
            return true;
        if (parent.createDirectory())
            return true;

        error = "Failed to create destination directory.";
        return false;
    }

    static bool hasPdfHeader(const juce::File& file)
    {
        std::unique_ptr<juce::FileInputStream> stream(file.createInputStream());
        if (stream == nullptr || !stream->openedOk() || stream->getTotalLength() < 4)
            return false;

        char header[4] = {};
        const auto bytes = stream->read(header, 4);
        return bytes == 4 && std::memcmp(header, "%PDF", 4) == 0;
    }
};
