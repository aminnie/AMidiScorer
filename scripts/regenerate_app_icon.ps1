# Regenerates src/resources/icons/app-icon-master.png (1024x1024 ARGB).
# contentScale enlarges the score artwork inside the frame (1.0 = baseline fill).
param(
    [double]$contentScale = 1.25
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

function New-RoundedRectPath([System.Drawing.Rectangle]$rect, [int]$radius)
{
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $path.AddArc($rect.X, $rect.Y, $radius, $radius, 180, 90)
    $path.AddArc($rect.Right - $radius, $rect.Y, $radius, $radius, 270, 90)
    $path.AddArc($rect.Right - $radius, $rect.Bottom - $radius, $radius, $radius, 0, 90)
    $path.AddArc($rect.X, $rect.Bottom - $radius, $radius, $radius, 90, 90)
    $path.CloseFigure()
    return $path
}

$size = 1024
$fmt = [System.Drawing.Imaging.PixelFormat]::Format32bppArgb
$bmp = New-Object System.Drawing.Bitmap $size, $size, $fmt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = 'AntiAlias'
$g.Clear([System.Drawing.Color]::FromArgb(0, 0, 0, 0))

# Outer transparent margin around the framed icon (smaller = larger on taskbar).
$frameInset = 28
$frameRect = New-Object System.Drawing.Rectangle $frameInset, $frameInset, ($size - ($frameInset * 2)), ($size - ($frameInset * 2))
$framePen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 70, 78, 92)), 28
$path = New-RoundedRectPath $frameRect 64
$sheetBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 252, 248, 236))
$g.FillPath($sheetBrush, $path)
$g.DrawPath($framePen, $path)

$ink = [System.Drawing.Color]::FromArgb(255, 28, 36, 52)
$centerX = $frameRect.X + ($frameRect.Width / 2.0)
$centerY = $frameRect.Y + ($frameRect.Height / 2.0)
$g.TranslateTransform([single]$centerX, [single]$centerY)
$g.ScaleTransform([single]$contentScale, [single]$contentScale)
$g.TranslateTransform([single](-$centerX), [single](-$centerY))

$staffPen = New-Object System.Drawing.Pen $ink, 16
$fill = New-Object System.Drawing.SolidBrush $ink

$staffLeft = $frameRect.X + 48
$staffRight = $frameRect.Right - 48
$staffTop = $frameRect.Y + 105
$staffBottom = $frameRect.Bottom - 105
$lineGap = ($staffBottom - $staffTop) / 4.0
for ($i = 0; $i -lt 5; $i++) {
    $y = [int]($staffTop + ($i * $lineGap))
    $g.DrawLine($staffPen, $staffLeft, $y, $staffRight, $y)
}

$clefPen = New-Object System.Drawing.Pen $ink, 24
$clefPen.StartCap = 'Round'
$clefPen.EndCap = 'Round'
$cx = $staffLeft + 70
$cy = [int]($staffTop + $lineGap * 2)
$g.DrawArc($clefPen, $cx - 34, $cy - 138, 104, 104, 200, 220)
$g.DrawArc($clefPen, $cx - 12, $cy - 46, 82, 82, 180, 180)
$g.DrawArc($clefPen, $cx + 12, $cy + 22, 72, 72, 0, 180)
$g.FillEllipse($fill, $cx + 42, $cy + 78, 30, 30)

$noteR = 36
$n1x = $staffLeft + 360
$n2x = $staffLeft + 500
$n1y = [int]($staffTop + $lineGap * 2.5 - $noteR)
$n2y = [int]($staffTop + $lineGap * 1.5 - $noteR)
$g.FillEllipse($fill, $n1x, $n1y, $noteR * 2, $noteR * 2)
$g.FillEllipse($fill, $n2x, $n2y, $noteR * 2, $noteR * 2)
$beamY = [int]($staffTop + $lineGap * 1.02)
$g.FillRectangle($fill, $n1x + 52, $beamY, $n2x - $n1x + 42, 20)
$g.FillRectangle($fill, $n2x + 58, $beamY, 20, [int]($lineGap * 2.2))

$g.ResetTransform()

$out = Join-Path (Split-Path $PSScriptRoot -Parent) 'src\resources\icons\app-icon-master.png'
$bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)

$g.Dispose()
$bmp.Dispose()
$framePen.Dispose()
$staffPen.Dispose()
$fill.Dispose()
$sheetBrush.Dispose()
$clefPen.Dispose()
$path.Dispose()

Write-Output "Wrote $out (contentScale=$contentScale)"
