#include <iostream>
#include <cmath>
#include <tuple>

#include <stdio.h>

#include <QtWidgets>
#include <QApplication>
#include <QMainWindow>

#include "mengele.h"

struct ColorHsl
{
    double hue;
    double saturation;
    double lumi;
};
struct ColorRgb
{
    double red;
    double green;
    double blue;
};
struct Params
{
    uint32_t maxIters = 100;
    uint32_t roundsPerRound = 123;
    real x = 0.0, y = 0.0;
    real zoom = 1;

    unsigned round = 0;
    int w = 1280, h = 720;

    bool record = false;
} c_start;

ColorRgb hslToRgb(const ColorHsl &hsl);

class Ikkuna
    : public QWidget
{
public:

    Ikkuna(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        m_renderObj = new QLabel(this);
        QHBoxLayout* layout = new QHBoxLayout;

        setFixedSize(1280, 720);
        layout->addWidget(m_renderObj);
        setLayout(layout);

        ffmpeg = popen(
            "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgra "
            "-s 1280x720 -r 60 -i - -f mp4 -q:v 1 -an "
            "-vcodec mpeg4 output.mp4", "w");
        
        img = QImage(p.w, p.h,  QImage::Format_ARGB32);
    }

    Params p = c_start;

    Mengele mengele;

    FILE* ffmpeg;

    QImage img;//(p.w, p.h,  QImage::Format_ARGB32);

private slots:

    void keyPressEvent(QKeyEvent *event) override
    {
        std::cout << event->text().toStdString() << "\n";
        if(event->key() == Qt::Key_Z)
        {
            p.zoom = p.zoom*0.95;
        }
        if(event->key() == Qt::Key_X)
        {
            const auto tmp = p.zoom;
            p.zoom = p.zoom*1.05;
            if (p.zoom == tmp) p.zoom++;
        }

        if(event->key() == Qt::Key_Q)
        {
            p.maxIters = p.maxIters*0.95;
            m_colorMap.clear();
        }
        if(event->key() == Qt::Key_E)
        {
            p.maxIters = p.maxIters*1.05;
            m_colorMap.clear();
        }

        if(event->key() == Qt::Key_R)
        {
            p.roundsPerRound--;
        }
        if(event->key() == Qt::Key_T)
        {
            p.roundsPerRound++;
        }

        if(event->key() == Qt::Key_W)
        {
            p.y = p.y - 0.10*p.zoom;
        }
        if(event->key() == Qt::Key_S)
        {
            p.y = p.y + 0.10*p.zoom;
        }

        if(event->key() == Qt::Key_D)
        {
            p.x = p.x + 0.10*p.zoom;
        }
        if(event->key() == Qt::Key_A)
        {
            p.x = p.x - 0.10*p.zoom;
        }

        if(event->key() == Qt::Key_C)
        {
            fflush(ffmpeg);
            p.record = !p.record;
        }

        std::cout 
            << "iters: " << p.maxIters << "\n"
            << "rounds per round: " << p.roundsPerRound << "\n"
            << "maxIters: " << p.maxIters << "\n"
            << "x: " << p.x << "\n"
            << "y: " << p.y << "\n"
            << "zoom: " << p.zoom << "\n"
            << "record: " << p.record << "\n"
        ;
    }

    void paintEvent(QPaintEvent* event) override
    {
        p.round++;
        
        const Frame frame = mengele.calcFrame(
            FrameParams{
                .x = p.x,
                .y = p.y,
                .zoom = p.zoom,
                .width = p.w,
                .height = p.h,
                .maxIters = p.maxIters,
            }
        );

        if (m_colorMap.empty())
        {
            m_colorMap.resize(p.maxIters+1);

            // H needs to bee between 0.0 and 1.0 when iter is
            // between 0 and maxIters
            const real S = 1.0; //max saturation
            const real L = 0.5; //50% light

            #pragma omp parallel for
            for (size_t i = 0; i < p.maxIters; i++)
            {
                //const real H = std::log2l((real)i) / std::log2f((real)p.maxIters);
                const real H = (real)i / (real)p.maxIters;

                m_colorMap[i] = Color{
                    .h = H,
                    .s = S,
                    .l = L,
                };
            }
            m_colorMap[p.maxIters] = Color{
                .h = 0.0,
                .s = 0.0,
                .l = 0.0,
            };
        }

        auto colorTrans2 = [this](uint32_t it)
        {
            QColor col;
            const auto H = m_colorMap.at((it+p.round) % p.maxIters).h;
            const auto S = m_colorMap.at(it).s;
            const auto L = m_colorMap.at(it).l;

            const auto rgb = hslToRgb(ColorHsl{
                .hue = H,
                .saturation = S,
                .lumi = L,
            });
            //col.setHslF(h, s, l);
            col.setRed(rgb.red);
            col.setGreen(rgb.green);
            col.setBlue(rgb.blue);
            return col;
        };

        /* auto colorTrans1 = [](uint32_t it)
        {
            QColor col = Qt::black;
            if(it < maxIters)
            {
                float val = (float)it / maxIters;
                col.setHslF(std::fmod(val + val * round * 0.1f, 1.0f), 1.0f, val);
            }
            return col;
        };

        std::vector<QColor> colmap;

        colmap.resize(2000);

        for(auto i = 0; i < 2000; i++)
        {
            colmap[i] = colorTrans1(i);
        } */
        
        #pragma omp parallel for
        for (unsigned i = 0; i < p.h; i++)
        {
            for (unsigned j = 0; j < p.w; j++)
            {
                auto it = frame.at(i * p.w + j);

                img.setPixelColor(j, i, colorTrans2(it));
            }
        }

        m_renderObj->setPixmap(QPixmap::fromImage(img.scaled(size())));
        m_renderObj->update();

        if (p.record) fwrite(img.bits(), 1, img.sizeInBytes(), ffmpeg);
    }

private:

    QPixmap m_pixmap;
    QLabel* m_renderObj;
    std::vector<Color> m_colorMap;
};

int main(int argc, char** argv)
{
    std::cout << "Hello, Mengels!\n";

    QApplication appiukko(argc, argv);
    Ikkuna akkuna;
    akkuna.show();

    appiukko.exec();

    return EXIT_SUCCESS;
}

ColorRgb hslToRgb(const ColorHsl &hsl) 
{
    double red;
    double green;
    double blue;

    if (hsl.saturation == 0.0f) {
        red = hsl.lumi * 255.0;
        green = hsl.lumi * 255.0;
        blue = hsl.lumi * 255.0;
    } else {
        float chroma = (1 - std::abs(2 * hsl.lumi - 1)) * hsl.saturation;
        float hue = hsl.hue * 6.0f;
        float x = chroma * (1 - std::abs(std::fmod(hue, 2.0f) - 1));
        float magnitude = hsl.lumi - chroma / 2;

        auto hue_to_rgb = [](float chroma, float x, float hue) -> auto {
            if (hue < 0.0f)
                return std::make_tuple(0.0f, 0.0f, 0.0f);
            int hue_floor = static_cast<int>(std::floor(hue));
            switch (hue_floor) {
            case 0:
                return std::make_tuple(chroma, x, 0.0f);
            case 1:
                return std::make_tuple(x, chroma, 0.0f);
            case 2:
                return std::make_tuple(0.0f, chroma, x);
            case 3:
                return std::make_tuple(0.0f, x, chroma);
            case 4:
                return std::make_tuple(x, 0.0f, chroma);
            case 5:
                return std::make_tuple(chroma, 0.0f, x);
            default:
                return std::make_tuple(0.0f, 0.0f, 0.0f);
            }
        };
        std::tie(red, green, blue) = hue_to_rgb(chroma, x, hue);
        red = (red + magnitude) * 255.0f;
        green = (green + magnitude) * 255.0f;
        blue = (blue + magnitude) * 255.0f;
    }

    return {red, green, blue};
}