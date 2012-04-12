#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/FFT.hpp>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <ctime>

using namespace std;
using namespace KlayGE;

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp()
		: App3DFramework("FFTLensEffectsGen")
	{
	}

	void DoUpdateOverlay()
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	std::string src_name("input.dds");

	if (argc > 1)
	{
		src_name = argv[1];
	}

	ResLoader::Instance().AddPath("../../../bin");

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	Context::Instance().Config(context_cfg);
	
	EmptyApp app;
	app.Create();

	
	int const WIDTH = 512;
	int const HEIGHT = 512;

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	TexturePtr empty_tex;
	{
		std::vector<uint8_t> zero_data(WIDTH * HEIGHT, 0);
		ElementInitData resized_data;
		resized_data.data = &zero_data[0];
		resized_data.row_pitch = WIDTH * sizeof(uint8_t);
		resized_data.slice_pitch = WIDTH * HEIGHT * sizeof(uint8_t);
		empty_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &resized_data);
	}

	TexturePtr pattern_raw = SyncLoadTexture(src_name, EAH_CPU_Read);
	int width = static_cast<int>(pattern_raw->Width(0));
	int height = static_cast<int>(pattern_raw->Height(0));
	if (pattern_raw->Format() != EF_ABGR8)
	{
		TexturePtr pattern_refmt = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read, NULL);
		pattern_raw->CopyToSubTexture2D(*pattern_refmt, 0, 0, 0, 0, width, height, 0, 0, 0, 0, width, height);
		pattern_raw = pattern_refmt;
	}

	std::vector<float> pattern_r_raw(WIDTH * HEIGHT, 0);
	std::vector<float> pattern_g_raw(WIDTH * HEIGHT, 0);
	std::vector<float> pattern_b_raw(WIDTH * HEIGHT, 0);

	float sum[3] = { 0, 0, 0 };
	std::vector<float> pattern_real(WIDTH * HEIGHT * 4);
	
	{
		Texture::Mapper mapper(*pattern_raw, 0, 0, TMA_Read_Only, 0, 0, width, height);

		uint8_t const * p = mapper.Pointer<uint8_t>();
		uint32_t const pitch = mapper.RowPitch() / sizeof(uint8_t);
		for (int y = 0; y < height / 2; ++ y)
		{
			int start_y = HEIGHT - height / 2;

			for (int x = 0; x < width / 2; ++ x)
			{
				int start_x = WIDTH - width / 2;

				float r = p[y * pitch + x * 4 + 0] / 255.0f;
				float g = p[y * pitch + x * 4 + 1] / 255.0f;
				float b = p[y * pitch + x * 4 + 2] / 255.0f;
				float a = p[y * pitch + x * 4 + 3] / 255.0f;
				if (a < 1e-6f)
				{
					r = g = b = 0;
				}

				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 0] = r;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 1] = g;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 2] = b;

				sum[0] += r;
				sum[1] += g;
				sum[2] += b;
			}
			for (int x = width / 2; x < width; ++ x)
			{
				int start_x = -static_cast<int>(width / 2);

				float r = p[y * pitch + x * 4 + 0] / 255.0f;
				float g = p[y * pitch + x * 4 + 1] / 255.0f;
				float b = p[y * pitch + x * 4 + 2] / 255.0f;
				float a = p[y * pitch + x * 4 + 3] / 255.0f;
				if (a < 1e-6f)
				{
					r = g = b = 0;
				}

				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 0] = r;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 1] = g;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 2] = b;
				
				sum[0] += r;
				sum[1] += g;
				sum[2] += b;
			}
		}
		for (int y = height / 2; y < height; ++ y)
		{
			int start_y = -static_cast<int>(height / 2);

			for (int x = 0; x < width / 2; ++ x)
			{
				int start_x = WIDTH - width / 2;

				float r = p[y * pitch + x * 4 + 0] / 255.0f;
				float g = p[y * pitch + x * 4 + 1] / 255.0f;
				float b = p[y * pitch + x * 4 + 2] / 255.0f;
				float a = p[y * pitch + x * 4 + 3] / 255.0f;
				if (a < 1e-6f)
				{
					r = g = b = 0;
				}

				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 0] = r;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 1] = g;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 2] = b;
				
				sum[0] += r;
				sum[1] += g;
				sum[2] += b;
			}
			for (int x = width / 2; x < width; ++ x)
			{
				int start_x = -static_cast<int>(width / 2);

				float r = p[y * pitch + x * 4 + 0] / 255.0f;
				float g = p[y * pitch + x * 4 + 1] / 255.0f;
				float b = p[y * pitch + x * 4 + 2] / 255.0f;
				float a = p[y * pitch + x * 4 + 3] / 255.0f;
				if (a < 1e-6f)
				{
					r = g = b = 0;
				}

				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 0] = r;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 1] = g;
				pattern_real[((y + start_y) * WIDTH + (x + start_x)) * 4 + 2] = b;
				
				sum[0] += r;
				sum[1] += g;
				sum[2] += b;
			}
		}
	}

	float max_sum = std::max(std::max(sum[0], sum[1]), sum[2]) * 0.0075f;

	for (uint32_t y = 0; y < HEIGHT; ++ y)
	{
		for (uint32_t x = 0; x < WIDTH; ++ x)
		{
			pattern_real[(y * WIDTH + x) * 4 + 0] /= max_sum;
			pattern_real[(y * WIDTH + x) * 4 + 1] /= max_sum;
			pattern_real[(y * WIDTH + x) * 4 + 2] /= max_sum;
			pattern_real[(y * WIDTH + x) * 4 + 3] = 1;
		}
	}

	ElementInitData pattern_real_data;
	pattern_real_data.data = &pattern_real[0];
	pattern_real_data.row_pitch = WIDTH * sizeof(float) * 4;
	pattern_real_data.slice_pitch = WIDTH * HEIGHT * sizeof(float) * 4;
	TexturePtr real_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &pattern_real_data);

	TexturePtr pattern_real_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	TexturePtr pattern_imag_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	GPUFFT fft(WIDTH, HEIGHT, true);
	fft.Execute(pattern_real_tex, pattern_imag_tex, real_tex, empty_tex);

	SaveTexture(pattern_real_tex, "lens_effects_real.dds");
	SaveTexture(pattern_imag_tex, "lens_effects_imag.dds");
}