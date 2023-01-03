#include <iostream>
#include <cmath>
#include <tuple>
#include <deque>
#include <thread>
#include <future>

#include <stdio.h>

#include <QtWidgets>
#include <QApplication>
#include <QMainWindow>

#include <fmt/core.h>

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
    int w = 1280, h = 720;
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

FILE* initFfmpeg()
{
    return popen(
        fmt::format(
            "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgra "
            "-s {}x{} -r 25 -i - -f mp4 -q:v 1 -an "
            "-vcodec mpeg4 output.mp4",
            c.w,
            c.h
        ).c_str(), 
        "w"
    );
}

std::future<Frame> asyncMengele(
    Params& params,
    const StaticParams& sParams,
    Mengele& mengele
)
{
    params.round = params.round + params.roundsPerRound;
    params.zoom = params.zoom * params.zoomPerRound;

    return std::async(
        std::launch::async,
        [&]()
        {
            return mengele.calcFrame(
                FrameParams{
                    .x = params.x,
                    .y = params.y,
                    .zoom = params.zoom,
                    .width = sParams.w,
                    .height = sParams.h,
                    .maxIters = params.maxIters,
                }
            );
        }
    );
}

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

        m_ffmpeg = initFfmpeg();
        
        m_img = QImage(c.w, c.h,  QImage::Format_ARGB32);

        m_futureFrame = asyncMengele(p, c, m_mengele);
    }

    Params p = c_start;

    Mengele m_mengele;

    std::future<Frame> m_futureFrame;
    Frame m_frame;

    FILE* m_ffmpeg;

    QImage m_img;

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
            p.zoom = p.zoom*1.01;
        }
        if(event->key() == Qt::Key_I)
        {
            p.zoom = 1.0; 
        }
        if(event->key() == Qt::Key_O)
        {
            p.zoomPerRound = p.zoomPerRound*0.99;
        }
        if(event->key() == Qt::Key_P)
        {
            p.zoomPerRound = p.zoomPerRound*1.01;
        }
        if(event->key() == Qt::Key_L)
        {
            p.zoomPerRound = 1.0; 
        }
        if(event->key() == Qt::Key_Y)
        {
            p.hueX = p.hueX*0.99;
        }
        if(event->key() == Qt::Key_U)
        {
            p.hueX = p.hueX*1.01;
        }
        if(event->key() == Qt::Key_K)
        {
            p.hueX = 1.0; 
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
            fflush(m_ffmpeg);
            p.record = !p.record;
            if (p.record)
            {
                std::cout << "Recording!\n";
            }
        }
        if(event->key() == Qt::Key_V)
        {
            fflush(m_ffmpeg);
            pclose(m_ffmpeg);
            m_ffmpeg = initFfmpeg();
            std::cout << "Recording reseted!\n";
        }

        std::cout 
            << "maxIters: " << p.maxIters << "\n"
            << "roundsPerRound: " << p.roundsPerRound << "\n"
            << "round: " << p.round  % p.maxIters<< "\n"
            << "x: " << p.x << " y: " << p.y << "\n"
            << "zoom: " << p.zoom << " zoomPerRound: " << p.zoomPerRound << "\n"
            << "hueX: " << p.hueX << "\n"
            << "record: " << p.record << "\n"
        ;
    }

    void paintEvent(QPaintEvent* event) override
    {
        m_frame = m_futureFrame.get();
        m_futureFrame = asyncMengele(p, c, m_mengele);

        if (m_colorMap.size() != p.maxIters)
        {
            m_colorMap.resize(p.maxIters);

            // H needs to bee between 0.0 and 1.0 when iter is
            // between 0 and maxIters
            const real S = 1.0; //max saturation
            const real L = 0.5; //50% light

            #pragma omp parallel for
            for (size_t i = 0; i < p.maxIters; i++)
            {
                //const real H = std::log((real)i) / std::log((real)p.maxIters);
                const real H = (real)(i*i) / (real)(p.maxIters * p.maxIters);

                m_colorMap[i] = Color{
                    .h = H,
                    .s = S,
                    .l = L,
                };
            }
            
            m_colorMap[p.maxIters - 1] = Color{
                .h = 0.0,
                .s = 0.0,
                .l = 0.0,
            };
        }

        auto colorTrans2 = [this](uint32_t it)
        {
            if (it >= m_colorMap.size())
            {
                return QColor(Qt::black);
            }
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

        //#pragma omp parallel for
        for (unsigned i = 0; i < c.h; i++)
        {
            for (unsigned j = 0; j < c.w; j++)
            {
                auto it = m_frame.at(i * c.w + j);

                m_img.setPixelColor(j, i, colorTrans2(it));
            }
        }

        std::thread renderObjT(
            [this]()
            {
                m_renderObj->setPixmap(QPixmap::fromImage(m_img));
                m_renderObj->update();
            }
        );

        std::thread m_ffmpegT(
            [this]()
            {
                if (p.record) fwrite(m_img.bits(), 1, m_img.sizeInBytes(), m_ffmpeg);
            }
        );

        renderObjT.join();
        m_ffmpegT.join();
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

    if (saturation == 0.0) {
        red = lumi * 255.0;
        green = lumi * 255.0;
        blue = lumi * 255.0;
    } else {
        double chroma = (1 - std::abs(2 * lumi - 1)) * saturation;
        double hue_asd = hue * 6.0;
        double x = chroma * (1 - std::abs(std::fmod(hue, 2.0) - 1));
        double magnitude = lumi - chroma / 2;

        auto hue_to_rgb = [](double chroma, double x, double hue) -> auto {
            if (hue < 0.0)
                return std::make_tuple(0.0, 0.0, 0.0);
            int hue_floor = static_cast<int>(std::floor(hue));
            switch (hue_floor) {
            case 0:
                return std::make_tuple(chroma, x, 0.0);
            case 1:
                return std::make_tuple(x, chroma, 0.0);
            case 2:
                return std::make_tuple(0.0, chroma, x);
            case 3:
                return std::make_tuple(0.0, x, chroma);
            case 4:
                return std::make_tuple(x, 0.0, chroma);
            case 5:
                return std::make_tuple(chroma, 0.0, x);
            default:
                return std::make_tuple(0.0, 0.0, 0.0);
            }
        };
        std::tie(red, green, blue) = hue_to_rgb(chroma, x, hue_asd);
        red = (red + magnitude) * 255.0;
        green = (green + magnitude) * 255.0;
        blue = (blue + magnitude) * 255.0;
    }

    return {red, green, blue};
}