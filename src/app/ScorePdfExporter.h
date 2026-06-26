#pragma once

#include <JuceHeader.h>
#include <vector>
#include "../notation/ScoreModel.h"
#include "../notation/ScoreRenderer.h"
#include "../notation/SimplePdfWriter.h"

class ScorePdfExporter
{
public:
    struct StaffExportLane
    {
        const ScoreModel* model = nullptr;
        const ScoreRenderer* renderer = nullptr;
    };

    struct ExportOptions
    {
        juce::String title;
        juce::String subtitle;
        int barsPerRow = 4;
    };

    static bool exportToFile(const juce::File& destination,
                             const std::vector<StaffExportLane>& lanes,
                             const ExportOptions& options,
                             juce::String& error,
                             int* exportedPageCount = nullptr)
    {
        error.clear();
        if (exportedPageCount != nullptr)
            *exportedPageCount = 0;

        std::vector<StaffExportLane> activeLanes;
        activeLanes.reserve(lanes.size());
        for (const auto& lane : lanes)
        {
            if (lane.model != nullptr && lane.renderer != nullptr && !lane.model->empty())
                activeLanes.push_back(lane);
        }

        if (activeLanes.empty())
        {
            error = "No staff notation to export.";
            return false;
        }

        int firstBar = std::numeric_limits<int>::max();
        int lastBar = 1;
        for (const auto& lane : activeLanes)
        {
            firstBar = juce::jmin(firstBar, lane.model->getFirstBar());
            lastBar = juce::jmax(lastBar, lane.model->getLastBar());
        }

        const int barsPerRow = juce::jmax(1, options.barsPerRow);
        if (lastBar < firstBar)
        {
            error = "No bar range available for PDF export.";
            return false;
        }

        juce::Array<juce::Image> pages;
        if (!renderPages(pages, activeLanes, options, firstBar, lastBar, barsPerRow, error))
            return false;

        if (!SimplePdfWriter::writeImagePages(destination, pages, error))
            return false;

        if (exportedPageCount != nullptr)
            *exportedPageCount = pages.size();

        return true;
    }

private:
    static bool renderPages(juce::Array<juce::Image>& pages,
                            const std::vector<StaffExportLane>& activeLanes,
                            const ExportOptions& options,
                            int firstBar,
                            int lastBar,
                            int barsPerRow,
                            juce::String& error)
    {
        constexpr int pageWidth = 1240;
        constexpr int pageHeight = 1754;
        constexpr int pageMargin = 54;
        constexpr int laneHeight = 220;
        constexpr int laneGap = 8;
        constexpr int rowGap = 14;
        constexpr int titleHeight = 46;

        const bool darkScheme = activeLanes.front().renderer->getColorScheme() == ScoreRenderer::ColorScheme::dark;
        const auto pageBackground = darkScheme ? juce::Colour(0xff0f1218) : juce::Colours::white;
        const auto titleColour = darkScheme ? juce::Colours::whitesmoke : juce::Colours::black;

        const juce::Rectangle<int> pageBounds(0, 0, pageWidth, pageHeight);
        const juce::Rectangle<int> contentBounds = pageBounds.reduced(pageMargin);
        const int rowHeight = static_cast<int>(activeLanes.size()) * laneHeight
                              + juce::jmax(0, static_cast<int>(activeLanes.size()) - 1) * laneGap;
        const int minimumContentHeight = rowHeight + rowGap;
        if (minimumContentHeight > contentBounds.getHeight())
        {
            error = "Export layout does not fit page height.";
            return false;
        }

        auto createPage = [&](bool includeTitle)
        {
            juce::Image pageImage(juce::Image::RGB, pageWidth, pageHeight, true);
            juce::Graphics g(pageImage);
            g.fillAll(pageBackground);
            if (includeTitle)
            {
                auto titleArea = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY(), contentBounds.getWidth(), titleHeight);
                g.setColour(titleColour);
                g.setFont(juce::Font(juce::FontOptions(21.0f, juce::Font::bold)));
                const auto titleText = options.title.isNotEmpty() ? options.title : juce::String("MidiScorer Score Export");
                g.drawFittedText(titleText, titleArea, juce::Justification::centredLeft, 1);
                if (options.subtitle.isNotEmpty())
                {
                    g.setFont(juce::Font(juce::FontOptions(14.0f)));
                    g.drawFittedText(options.subtitle, titleArea.withTrimmedTop(24), juce::Justification::centredLeft, 1);
                }
            }
            return pageImage;
        };

        juce::Image pageImage = createPage(true);
        bool firstPage = true;
        int cursorY = contentBounds.getY() + titleHeight + 6;

        for (int barStart = firstBar; barStart <= lastBar; barStart += barsPerRow)
        {
            const int nextRowBottom = cursorY + rowHeight;
            if (nextRowBottom > contentBounds.getBottom())
            {
                pages.add(pageImage);
                pageImage = createPage(false);
                firstPage = false;
                cursorY = contentBounds.getY();
            }

            juce::Graphics pageGraphics(pageImage);
            int laneTop = cursorY;
            for (const auto& lane : activeLanes)
            {
                const int barEnd = juce::jmin(lastBar, barStart + barsPerRow - 1);
                const auto bars = lane.model->getBarsInRange(barStart, barEnd);
                juce::Rectangle<int> laneBounds(contentBounds.getX(), laneTop, contentBounds.getWidth(), laneHeight);

                ScoreRenderer::BarStripPaintOptions paintOptions;
                paintOptions.highlightCurrentBar = false;
                paintOptions.includeLiveChordMarker = false;
                paintOptions.currentBarForHighlight = barStart;
                lane.renderer->paintBarStrip(pageGraphics, laneBounds, bars, paintOptions);

                laneTop += laneHeight + laneGap;
            }

            cursorY = laneTop - laneGap + rowGap;
            if (firstPage)
                firstPage = false;
        }

        pages.add(pageImage);
        if (pages.isEmpty())
        {
            error = "No pages were generated for PDF export.";
            return false;
        }

        return true;
    }
};
