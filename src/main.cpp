#include <iostream>
#include <cmath>
#include <new>
#include <string>
#include <tuple>
#include <deque>
#include <thread>
#include <future>

#include <stdio.h>

#include <QtWidgets>
#include <QApplication>
#include <QMainWindow>

#include <fmt/core.h>

#include "mandelbrot_oneapi.h"

//#include "lemonav.h"
//cmake .. -DCMAKE_C_COMPILER=icp -DCMAKE_CXX_COMPILER=icpx

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
	//int w = 1280, h = 720;
    int w = 1920, h = 1080;
    //int w = 3840, h = 2160;
} c;

struct Params
{
    //uint32_t maxIters = 6543;
    uint32_t maxIters = 200;
    int roundsPerRound = 0.0;
    float round = 0.0;

    real x = 1, y = 1,
        xX = 1.0, yX = 1.0;
    real zoom = 1, zoomPerRound = 1.0,
        zoomCur = 1.0, zoomCurPerRound = 1.0;
    real hueX = 1.0, hueMin = 0.0, hueMax = 50.0, huePlus = 0.0;
	real a = 1.0, b = 1.0, c = 1.0, d = 1.0, e = 1.0;

	int paramToSet = 0;

    int outputCount = 0;

    bool record = false;
} p_start;

FILE* initFfmpeg(Params& p)
{   // ffmpeg -i output0.mp4 -vcodec libaom-av1 -vf scale=1920:1080 -crf 28 -preset slow asd_s.mp4
    //ffmpeg -i output0.mp4 -vcodec libx265 -vf scale=1920:1080 -crf 28 -preset slow asd_s.mp4
    return popen(
        fmt::format(
            "ffmpeg -y -hwaccel cuda -hwaccel_output_format cuda "
			"-hwaccel_device 1 " // 1 is 1050ti
			"-f rawvideo "
			"-pix_fmt bgra "
            "-s {}x{} -r 10 "
			"-i - "
			//R"(-vf "format=yuv420p,hwupload_cuda,scale_npp=1920:1080:interp_algo=super" )"
			//"-vf scale=1280:720 "
			"-b:v 20M "
			//"-tune hq "
			"-preset p7 "
			"-b_ref_mode 0 "
            "-c:v hevc_nvenc "
			//"-c:v mpeg4 "
			"output{}.mp4",
            c.w,
            c.h,
            p.outputCount++
        ).c_str(),
        "w"
    );
}

std::future<uint32_t*> asyncMengele(
    Params& params,
    const StaticParams& sParams,
    Mandelbrot& mengele
)
{
    params.round += params.roundsPerRound;
    params.zoom *= params.zoomPerRound;
    params.zoomCur *= params.zoomCurPerRound;

    return std::async(
        std::launch::async,
        [&]()
        {
			mengele.initMandelbrot(
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
            return mengele.runMandelbrot();
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

        m_futureFrame = asyncMengele(p, c, m_mandelbrot);
    }

private slots:

    void keyPressEvent(QKeyEvent *event) override
    {
		std::string pSetStr;

        if(event->key() == Qt::Key_1) // change
        {
			p.paramToSet++;
            if (p.paramToSet == 3) p.paramToSet = 0;
        }
		real* param;
		switch (p.paramToSet)
		{
		case 0:
			param = &p.hueMin;
			pSetStr = "hueMin";
			break;
		case 1:
			param = &p.huePlus;
			pSetStr = "huePlus";
			break;
		case 2:
			param = &p.hueMax;
			pSetStr = "hueMax";
			break;
		case 3:
			param = &p.a;
			pSetStr = "a";
			break;
		case 4:
			param = &p.b;
			pSetStr = "b";
			break;
		case 5:
			param = &p.c;
			pSetStr = "c";
			break;
		case 6:
			param = &p.d;
			pSetStr = "d";
			break;
		case 7:
			param = &p.d;
			pSetStr = "d";
			break;
		case 8:
			param = &p.e;
			pSetStr = "e";
			break;


		default:
			break;
		}

        if(event->key() == Qt::Key_2)
        {
            *param = 0.0;
        }
        if(event->key() == Qt::Key_3)
        {
            *param -= 1.0;
        }
        if(event->key() == Qt::Key_4)
        {
            *param -= 0.1;
        }
        if(event->key() == Qt::Key_5)
        {
            *param -= 0.01;
        }
        if(event->key() == Qt::Key_6)
        {
            *param += 0.01;
        }
        if(event->key() == Qt::Key_7)
        {
            *param += 0.1;
        }
        if(event->key() == Qt::Key_8)
        {
            *param += 1.0;
        }
		if (param == &p.huePlus
			&& (event->key() == Qt::Key_1
				|| event->key() == Qt::Key_2
				|| event->key() == Qt::Key_3
				|| event->key() == Qt::Key_4
				|| event->key() == Qt::Key_5))
		{
			m_colorMap.clear();
		}

        if(event->key() == Qt::Key_Z) // ZOOM
        {
            p.zoom *= 0.99;
        }
        if(event->key() == Qt::Key_X)
        {
            p.zoom *= 1.01;
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
            p.roundsPerRound = 0.0;
        }
        // if(event->key() == Qt::Key_G)
        // {
        //     p.roundsPerRound += 0.1;
        // }

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
			if(m_ffmpeg)
            	fflush(m_ffmpeg);
            p.record = !p.record;
            if (p.record)
            {
                std::cout << "Recording!\n";
            }
        }
        if(event->key() == Qt::Key_V) // reset
        {
			if(m_ffmpeg){
				fflush(m_ffmpeg);
				pclose(m_ffmpeg);
			}
			m_ffmpeg = initFfmpeg(p);
			std::cout << "Recording reseted!\n";
        }
        std::cout
            << "maxIters: " << p.maxIters << "\n"
            << "roundsPerRound: " << p.roundsPerRound << "\n"
            << "round: " << (int)p.round % p.maxIters<< "\n"
            << "x: " << p.x << " y: " << p.y << "\n"
            << "zoom: " << p.zoom << " zoomPerRound: " << p.zoomPerRound
                << " zoomCur: " << p.zoomCur << " zoomCurPerRound: " << p.zoomCurPerRound << "\n"
            << "hueX: " << p.hueX << " hueMin: " << p.hueMin << " huePlus: " << p.huePlus
				<< " hueMax: "  << p.hueMax << "\n"
			<< "a: " << p.a << " b: " << p.b << " c: " << p.c << " d: " << p.d << " e: " << p.e << "\n"
            << "record: " << p.record << "\n"
			<< "12345 sets " << pSetStr << "\n"
        ;
    }

    // template <typename Func>
    // float funcjuttu(
    //     const auto& i,
    //     const auto& max,
    //     const Func& func
    // )
    // {
    //     return (float)func(i) / (float)func(max);
    // }

    void paintEvent(QPaintEvent* event) override
    {
        uint32_t* buffer = m_futureFrame.get();
		//copy buffer to m_frame
		for (int i = 0; i < c.w*c.h; i++)
		{
			m_frame[i] = buffer[i];
		}
        m_futureFrame = asyncMengele(p, c, m_mandelbrot);




		// p.round += p.roundsPerRound;
		// p.zoom *= p.zoomPerRound;
		// p.zoomCur *= p.zoomCurPerRound;

		// m_mandelbrot.initMandelbrot(
		// 	FrameParams{
        //             .x = p.x,
        //             .y = p.y,
        //             .zoom = p.zoom,
        //             .zoomCur = p.zoomCur,
        //             .width = c.w,
        //             .height = c.h,
        //             .maxIters = p.maxIters,
        //         }
		// );

		// m_frame = m_mandelbrot.runMandelbrot();



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

            //#pragma omp parallel for
            for (size_t i = 0; i < p.maxIters; i++)
            {
				//const float H = p.huePlus + i*p.a + i*i*p.b + i*i*i*p.c + i*i*i*i*p.d;
                //const real H = (real)i / 128.0;
                const float H = i*std::sin(i / 1024.0)/128.0 + p.huePlus;
				//const float H = i/256.0 + p.huePlus;
                // const real H = funcjuttu(
                //     i,
                //     p.maxIters,
                //     [](const real a)
                //     {
                //         return a*std::sin(a / 10.0) - a;
                //     }
                // );

                m_colorMap[i] =
				ColorHsl{
                    .hue = H,
                    .saturation = S,
                    .lumi = L,
                };
            }

            m_colorMap[p.maxIters - 1] =
			ColorHsl{
                .hue = 0.0,
                .saturation = 0.0,
                .lumi = 0.0,
            };
        }

        auto colorTrans2 = [this](const uint32_t it)
        {
            if (it >= m_colorMap.size())
            {
                return QColor(Qt::black);
            }
            const auto H = m_colorMap[(int(p.hueX * it + p.hueX * p.round)) % p.maxIters].hue;
            const auto S = m_colorMap[it].saturation;
            const auto L =
                (H - p.huePlus) >= p.hueMin
				&& (H - p.huePlus) <= p.hueMax
					? m_colorMap[it].lumi
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

        //#pragma omp parallel for
        for (unsigned i = 0; i < c.h; i++)
        {
            for (unsigned j = 0; j < c.w; j++)
            {
                const auto it = m_frame[i * c.w + j];

                m_img.setPixelColor(j, i, colorTrans2(it));
            }
        }

		if (p.record)
		{
			if (ffmpegT.joinable()) ffmpegT.join();

			m_imgff = m_img.copy();

			ffmpegT = std::thread(
				[this]()
				{
					if(m_ffmpeg)
						fwrite(m_imgff.bits(), 1, m_imgff.sizeInBytes(), m_ffmpeg);
				}
			);
		}

        m_renderObj->setPixmap(QPixmap::fromImage(m_img));
        m_renderObj->update();
    }

private:

    Params p = p_start;

    //Mengele m_mengele;

    std::future<uint32_t*> m_futureFrame;

    //alignas(std::hardware_constructive_interference_size)

	Mandelbrot m_mandelbrot;
    //Frame m_frame;
	uint32_t* m_frame = new uint32_t[c.w * c.h];

    FILE* m_ffmpeg = nullptr;
	std::thread ffmpegT;

    //alignas(std::hardware_constructive_interference_size)
    QImage m_img;

	//alignas(std::hardware_constructive_interference_size)
    QImage m_imgff;

    //alignas(std::hardware_constructive_interference_size)
    std::vector<ColorHsl> m_colorMap;

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
    if (saturation == 0.0f)
	{
		return ColorRgb{
			.red = lumi * 255.0f,
			.green = lumi * 255.0f,
			.blue = lumi * 255.0f,
		};
    }
	else
	{
        auto hue_to_rgb = [](float chroma, float x, float hue) -> auto
		{
            if (hue < 0.0f) return std::make_tuple(0.0f, 0.0f, 0.0f);

            int hue_floor = static_cast<int>(std::floor(hue));
            switch (hue_floor)
			{
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

		hue = std::fmod(std::abs(hue), 1.0f);

        float chroma = (1.0f - std::abs(2.0f * lumi - 1.0f)) * saturation;
        float hue_asd = hue * 6.0f;
        float x = chroma * (1.0f - std::abs(std::fmod(hue_asd, 2.0f) - 1.0f));
        float magnitude = lumi - chroma / 2.0f;

		float red;
		float green;
		float blue;

        std::tie(red, green, blue) = hue_to_rgb(chroma, x, hue_asd);

		return ColorRgb{
			.red = (red + magnitude) * 255.0f,
			.green = (green + magnitude) * 255.0f,
			.blue = (blue + magnitude) * 255.0f,
		};
    }
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
