#include <iostream>
#include <cmath>
#include <tuple>
#include <deque>
#include <thread>

#include <stdio.h>

#include <QtWidgets>
#include <QApplication>
#include <QMainWindow>

#include "mengele.h"

struct ColorRgb
{
    double red;
    double green;
    double blue;
};

struct ColorHsl
{
    double hue;
    double saturation;
    double lumi;

    ColorRgb toRgb();
};

struct StaticParams
{
    int w = 1920, h = 1080;
} c;

struct Params
{
    uint32_t maxIters = 100;
    uint32_t roundsPerRound = 0;
    uint32_t round = 0;

    real x = 0.0, y = 0.0;
    real zoom = 1.0, zoomPerRound = 1.0;
    real hueX = 1.0;

    bool record = false;
} c_start;

class Ikkuna
    : public QWidget
{
public:

    Ikkuna(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        m_renderObj = new QLabel(this);
        QHBoxLayout* layout = new QHBoxLayout;

        setFixedSize(c.w, c.h);
        layout->addWidget(m_renderObj);
        setLayout(layout);

        ffmpeg = popen(
            "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgra "
                "-s 1920x1080 -r 25 -i - -f mp4 -q:v 1 -an "
                "-vcodec mpeg4 output.mp4", 
            "w"
        );
        
        img = QImage(c.w, c.h,  QImage::Format_ARGB32);
    }

    Params p = c_start;

    Mengele mengele;

    FILE* ffmpeg;

    QImage img;

private slots:

    void keyPressEvent(QKeyEvent *event) override
    {
        std::cout << event->text().toStdString() << "\n";
        if(event->key() == Qt::Key_Z)
        {
            p.zoom = p.zoom*0.99;
        }
        if(event->key() == Qt::Key_X)
        {
            //const auto tmp = p.zoom;
            p.zoom = p.zoom*1.01;
            //if (p.zoom == tmp) p.zoom++;
        }
        if(event->key() == Qt::Key_O)
        {
            p.zoomPerRound = p.zoomPerRound*0.99;
        }
        if(event->key() == Qt::Key_P)
        {
            //const auto tmp = p.zoomPerRound;
            p.zoomPerRound = p.zoomPerRound*1.01;
            //if (p.zoomPerRound == tmp) p.zoomPerRound++;
        }
        if(event->key() == Qt::Key_I)
        {
            p.zoom = 1.0; 
        }
        if(event->key() == Qt::Key_Y)
        {
            p.hueX = p.hueX*0.99;
        }
        if(event->key() == Qt::Key_U)
        {
            //const auto tmp = p.hueX;
            p.hueX = p.hueX*1.01;
            //if (p.hueX == tmp) p.hueX++;
        }

        if(event->key() == Qt::Key_Q)
        {
            p.maxIters = p.maxIters*0.99;
            m_colorMap.clear();
        }
        if(event->key() == Qt::Key_E)
        {
            const auto tmp = p.maxIters;
            p.maxIters = p.maxIters*1.01;
            if (p.maxIters == tmp) p.maxIters++;
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
            p.y = p.y - 0.01*p.zoom;
        }
        if(event->key() == Qt::Key_S)
        {
            p.y = p.y + 0.01*p.zoom;
        }

        if(event->key() == Qt::Key_D)
        {
            p.x = p.x + 0.01*p.zoom;
        }
        if(event->key() == Qt::Key_A)
        {
            p.x = p.x - 0.01*p.zoom;
        }

        if(event->key() == Qt::Key_C)
        {
            fflush(ffmpeg);
            p.record = !p.record;
            if (p.record)
            {
                std::cout << "Recording!\n";
            }
        }
        if(event->key() == Qt::Key_V)
        {
            fflush(ffmpeg);
            pclose(ffmpeg);
            ffmpeg = popen(
                "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgra "
                    "-s 1920x1080 -r 25 -i - -f mp4 -q:v 1 -an "
                    "-vcodec mpeg4 output.mp4",
                "w"
            );
            std::cout << "Recording reseted!\n";
        }

        std::cout 
            << "maxIters: " << p.maxIters << "\n"
            << "roundsPerRound: " << p.roundsPerRound << "\n"
            << "round: " << p.round << "\n"
            << "x: " << p.x << " y: " << p.y << "\n"
            << "zoom: " << p.zoom << " zoomPerRound: " << p.zoomPerRound << "\n"
            << "record: " << p.record << "\n"
        ;
    }

    void paintEvent(QPaintEvent* event) override
    {
        p.round = p.round + p.roundsPerRound;

        p.zoom = p.zoom*p.zoomPerRound;

        Frame frame;

        std::thread frameT([this, &frame](){
        frame = mengele.calcFrame(
            FrameParams{
                .x = p.x,
                .y = p.y,
                .zoom = p.zoom,
                .width = c.w,
                .height = c.h,
                .maxIters = p.maxIters,
            }
        );
        }); 

        std::thread colorMapT([this](){
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
                const real H = std::log2l((real)i) / std::log2f((real)p.maxIters);
                //const real H = (real)i / (real)p.maxIters;

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
        });

        auto colorTrans2 = [this](uint32_t it)
        {
            const auto H = p.hueX * m_colorMap.at((it+p.round) % p.maxIters).h;
            const auto S = m_colorMap.at(it).s;
            const auto L = m_colorMap.at(it).l;

            const auto rgb = 
            ColorHsl{
                .hue = H,
                .saturation = S,
                .lumi = L,
            }.toRgb();

            QColor col;
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

        frameT.join();
        colorMapT.join();

        #pragma omp parallel for
        for (unsigned i = 0; i < c.h; i++)
        {
            for (unsigned j = 0; j < c.w; j++)
            {
                auto it = frame.at(i * c.w + j);

                img.setPixelColor(j, i, colorTrans2(it));
            }
        }

        const auto imgTmp = img;

        std::thread renderObjT([this](){
        m_renderObj->setPixmap(QPixmap::fromImage(img));
        m_renderObj->update();
        });

        std::thread ffmpegT([this, imgTmp](){
        if (p.record) fwrite(imgTmp.bits(), 1, imgTmp.sizeInBytes(), ffmpeg);
        });

        renderObjT.join();
        ffmpegT.join();
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

ColorRgb ColorHsl::toRgb() 
{
    double red;
    double green;
    double blue;

    if (saturation == 0.0f) {
        red = lumi * 255.0;
        green = lumi * 255.0;
        blue = lumi * 255.0;
    } else {
        float chroma = (1 - std::abs(2 * lumi - 1)) * saturation;
        float hue = hue * 6.0f;
        float x = chroma * (1 - std::abs(std::fmod(hue, 2.0f) - 1));
        float magnitude = lumi - chroma / 2;

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