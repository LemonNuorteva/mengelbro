#include <iostream>
#include <cmath>
#include <new>
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

#include "ffmpeg.h"

struct ColorRgb
{
    float red;
    float green;
    float blue;
};

struct ColorHsl
{
    float hue;
    float saturation;
    float lumi;

    ColorRgb toRgb();
};

struct StaticParams
{
    int w = 1280, h = 720;
    //int w = 3840, h = 2160;
} c;

struct Params
{
    uint32_t maxIters = 500;
    float roundsPerRound = 0.0;
    float round = 0.0;

    real x = 0, y = 0,
        xX = 1.0, yX = 1.0;
    real zoom = 1.0, zoomPerRound = 1.0,
        zoomCur = 1.0, zoomCurPerRound = 1.0;
    real hueX = 1.0, hueMin = 0.0;

    int outputCount = 0;

    bool record = false;
} p_start;

FILE* initFfmpeg(Params& p)
{   // ffmpeg -i output0.mp4 -vcodec libaom-av1 -vf scale=1920:1080 -crf 28 -preset slow asd_s.mp4
    //ffmpeg -i output0.mp4 -vcodec libx265 -vf scale=1920:1080 -crf 28 -preset slow asd_s.mp4
    return popen(
        fmt::format(
            "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgra "
            "-s {}x{} -r 10 -i - -f mp4 -q:v 1 -an "
            "-vcodec mpeg4 output{}.mp4",
            c.w,
            c.h,
            p.outputCount++
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
    params.round += params.roundsPerRound;
    params.zoom *= params.zoomPerRound;
    params.zoomCur *= params.zoomCurPerRound;

    return std::async(
        std::launch::async,
        [&]()
        {
            return mengele.calcFrame(
                FrameParams{
                    .x = params.x,
                    .y = params.y,
                    .zoom = params.zoom,
                    .zoomCur = params.zoomCur,
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

        m_ffmpeg = initFfmpeg(p);

        m_img = QImage(c.w, c.h,  QImage::Format_ARGB32);

        m_futureFrame = asyncMengele(p, c, m_mengele);
    }

private slots:

    void keyPressEvent(QKeyEvent *event) override
    {
        std::cout << event->text().toStdString() << "\n";
        if(event->key() == Qt::Key_Z) // ZOOM
        {
            p.zoom *=0.99;
        }
        if(event->key() == Qt::Key_X)
        {
            p.zoom *=1.01;
        }
        if(event->key() == Qt::Key_I)
        {
            p.zoom = 1.0;
        }
        if(event->key() == Qt::Key_O) // ZoomX
        {
            p.zoomPerRound *=0.99;
        }
        if(event->key() == Qt::Key_P)
        {
            p.zoomPerRound = p.zoomPerRound*1.01;
        }
        if(event->key() == Qt::Key_L)
        {
            p.zoomPerRound = 1.0;
        }
        if(event->key() == Qt::Key_N) // ZoomCurvX
        {
            p.zoomCurPerRound *= 0.99;
        }
        if(event->key() == Qt::Key_M)
        {
            p.zoomCurPerRound *= 1.01;
        }
        if(event->key() == Qt::Key_B)
        {
            p.zoomCurPerRound = 1.0;
        }
        if(event->key() == Qt::Key_Y) // HueX
        {
            p.hueX *= 0.99;
        }
        if(event->key() == Qt::Key_U)
        {
            p.hueX *= 1.01;
        }
        if(event->key() == Qt::Key_K)
        {
            p.hueX = 1.0;
        }
        if(event->key() == Qt::Key_1) // HueMin
        {
            p.hueMin += 1.0;
        }
        if(event->key() == Qt::Key_2)
        {
            p.hueMin -= 1.0;
        }
        if(event->key() == Qt::Key_3)
        {
            p.hueMin = 1.0;
        }

        if(event->key() == Qt::Key_Q) // MAX
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

        if(event->key() == Qt::Key_R) // Round
        {
            p.roundsPerRound -= 1.0;
        }
        if(event->key() == Qt::Key_T)
        {
            p.roundsPerRound += 1.0;
        }
        if(event->key() == Qt::Key_F)
        {
            p.roundsPerRound -= 0.1;
        }
        if(event->key() == Qt::Key_G)
        {
            p.roundsPerRound += 0.1;
        }

        if(event->key() == Qt::Key_W) // UP
        {
            p.y = p.y - 0.01*p.zoom;
        }
        if(event->key() == Qt::Key_S) // DOWN
        {
            p.y = p.y + 0.01*p.zoom;
        }

        if(event->key() == Qt::Key_D) // RIGHT
        {
            p.x = p.x + 0.01*p.zoom;
        }
        if(event->key() == Qt::Key_A) // LEFT
        {
            p.x = p.x - 0.01*p.zoom;
        }

        if(event->key() == Qt::Key_C) // start / stop
        {
            fflush(m_ffmpeg);
            p.record = !p.record;
            if (p.record)
            {
                std::cout << "Recording!\n";
            }
        }
        if(event->key() == Qt::Key_V) // reset
        {
            fflush(m_ffmpeg);
            pclose(m_ffmpeg);
            m_ffmpeg = initFfmpeg(p);
            std::cout << "Recording reseted!\n";
        }
    }

    template <typename Func>
    float funcjuttu(
        const auto& i,
        const auto& max,
        const Func& func
    )
    {
        return (float)func(i) / (float)func(max);
    }

    void paintEvent(QPaintEvent* event) override
    {
        std::cout
            << "maxIters: " << p.maxIters << "\n"
            << "roundsPerRound: " << p.roundsPerRound << "\n"
            << "round: " << (int)p.round  % p.maxIters<< "\n"
            << "x: " << p.x << " y: " << p.y << "\n"
            << "zoom: " << p.zoom << " zoomPerRound: " << p.zoomPerRound
                << "zoomCur: " << p.zoomCur << " zoomCurPerRound: " << p.zoomCurPerRound << "\n"
            << "hueX: " << p.hueX << ". hueMin:" << p.hueMin << "\n"
            << "record: " << p.record << "\n"
        ;
        m_frame = m_futureFrame.get();
        m_futureFrame = asyncMengele(p, c, m_mengele);

        // Frame asdfg = Mengele::convolute(
        //     c.h,
        //     c.w,
        //     m_frame,
        //     {
        //         {0.1, 0.1, 0.1},
        //         {0.1, 0.5, 0.1},
        //         {0.1, 0.1, 0.1}
        //     }
        // );

        if (m_colorMap.size() != p.maxIters)
        {
            m_colorMap.resize(p.maxIters);

            // H needs to bee between 0.0 and 1.0 when iter is
            const float S = 1.0; //max saturation
            const float L = 0.5; //50% light

            #pragma omp parallel for
            for (size_t i = 0; i < p.maxIters; i++)
            {
                //const real H = (real)i / 128.0;
                const float H = i*std::sin(i / 1024.0)/128.0;
                // const real H = funcjuttu(
                //     i,
                //     p.maxIters,
                //     [](const real a)
                //     {
                //         return a*std::sin(a / 10.0) - a;
                //     }
                // );

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

        auto colorTrans2 = [this](const uint32_t it)
        {
            if (it >= m_colorMap.size())
            {
                return QColor(Qt::black);
            }
            const auto H = m_colorMap[(int(p.hueX * it + p.round)) % p.maxIters].h;
            const auto S = m_colorMap[it].s;
            const auto L =
                H >= p.hueMin
                    ? m_colorMap[it].l
                    : 0.0f;

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

        #pragma omp parallel for
        for (unsigned i = 0; i < c.h; i++)
        {
            for (unsigned j = 0; j < c.w; j++)
            {
                const auto it = m_frame[i * c.w + j];

                m_img.setPixelColor(j, i, colorTrans2(it));
            }
        }

		if (ffmpegT.joinable()) ffmpegT.join();

        ffmpegT = std::thread(
            [this]()
            {
                if (p.record) fwrite(m_img.bits(), 1, m_img.sizeInBytes(), m_ffmpeg);
            }
        );

        m_renderObj->setPixmap(QPixmap::fromImage(m_img));
        m_renderObj->update();
    }

private:

    Params p = p_start;

    Mengele m_mengele;

    std::future<Frame> m_futureFrame;

    alignas(std::hardware_constructive_interference_size)
        Frame m_frame;

    FILE* m_ffmpeg;

    alignas(std::hardware_constructive_interference_size)
        QImage m_img;

    alignas(std::hardware_constructive_interference_size)
        std::vector<Color> m_colorMap;

	std::thread ffmpegT;

    QLabel* m_renderObj;
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
    float red;
    float green;
    float blue;

    hue = std::fmod(std::abs(hue), 1.0f);

    if (saturation == 0.0f) {
        red = lumi * 255.0f;
        green = lumi * 255.0f;
        blue = lumi * 255.0f;
    } else {
        float chroma = (1.0f - std::abs(2.0f * lumi - 1.0f)) * saturation;
        float hue_asd = hue * 6.0f;
        float x = chroma * (1.0f - std::abs(std::fmod(hue, 2.0f) - 1.0f));
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
        std::tie(red, green, blue) = hue_to_rgb(chroma, x, hue_asd);
        red = (red + magnitude) * 255.0f;
        green = (green + magnitude) * 255.0f;
        blue = (blue + magnitude) * 255.0f;
    }

    return {red, green, blue};
}

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
