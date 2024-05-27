#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C"
{
#include "vc.h"
}

void vc_timer(void)
{
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running)
	{
		running = true;
	}
	else
	{
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}

// Escrever o valor da resistência na imagem IVC por cima do blob
void vc_draw_resistance_value(IVC *srcdst, OVC *blob, int resistencia)
{
	// Declaração de variáveis
	char str[12];
	int x, y;

	// Cálculo da posição do texto
	x = blob->x;
	y = blob->y - blob->height / 3;

	// Converter a imagem IVC para uma imagem OpenCV
	cv::Mat frame(srcdst->height, srcdst->width, CV_8UC3, srcdst->data);

	// Escrever o valor da resistência na imagem
	snprintf(str, sizeof(str), "%d", resistencia);
	cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
	cv::putText(frame, str, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

	// Converter a imagem OpenCV para uma imagem IVC
	memcpy(srcdst->data, frame.data, srcdst->width * srcdst->height * 3);
}

int main(void)
{
	// Decralação de uma variável para capturar o vídeo
	cv::VideoCapture capture;
	// Escolha de qual método usar para ler
	int apiPreference = cv::CAP_AVFOUNDATION;
	// Captura do vídeo
	capture.open("/Users/zecastro/Desktop/VC_Trabalho/video_resistors.mp4", apiPreference);
	// Verificar foi possível abrir o ficheiro
	if (!capture.isOpened())
	{
		// Em caso de falha, imprime mensagem de erro para o terminal
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	// Estrutura para armazenar informações do vídeo
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;

	// Declaração de variável para melhor legibilidade do código
	std::string str;
	// Declaração de variável para armazenar a tecla premida pelo utilizador
	int key = 0;

	// Total de frames do vídeo
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	std::cout << "Total de frames: " << video.ntotalframes << std::endl;
	// Frame rate do vídeo
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	std::cout << "Frame rate: " << video.fps << std::endl;
	// Resolução do vídeo
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
	std::cout << "Resolução: " << video.width << "x" << video.height << std::endl;

	// Criação de uma janela
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);
	// Verificar se a janela foi criada
	if (!cv::getWindowProperty("VC - VIDEO", cv::WND_PROP_AUTOSIZE))
	{
		// Se a janela não for criada, imprime uma mensagem de erro e terminar o programa
		std::cerr << "Erro ao criar a janela!\n";
		return 1;
	}

	// Iniciar o cronómetro
	vc_timer();

	// Ciclo para capturar e processar cada frame do vídeo
	// Declaração de uma variável para armazenar o frame do vídeo
	cv::Mat frame;
	while (key != 'q')
	{
		// Verifica se o frame foi lido corretamente e se o número de frames lidos é o último
		if (!capture.read(frame) && video.nframe != video.ntotalframes)
		{
			// Em caso de erro, imprime mensagem de erro para o terminal
			std::cerr << "Erro: não foi possível ler o frame do vídeo!\n";
			break;
		}

		// Verifica se o frame está vazio
		if (frame.empty())
		{
			// Quebra o ciclo // O programa só deve entrar aqui se o vídeo acabar!
			break;
		}

		// Número do frame a processar
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		// Escrita de informações do vídeo no frame
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		// Criação de novas imagens IVC
		IVC *img[9];

		// TODO: Retirar a criação de novas imagens a cada processo, usar sempre a mesma.

		// Atribuição de valor a um imagem IVC
		img[0] = vc_image_new(video.width, video.height, 3, 255);

		// Cópia do frame do vídeo para a imagem IVC
		memcpy(img[0]->data, frame.data, video.width * video.height * 3);

		// Transformação de uma imagem BGR para RGB
		img[1] = vc_image_new(video.width, video.height, 3, 255);
		vc_bgr_to_rgb(img[0], img[1]);

		// Transformação de uma imagem RGB para HSV
		img[2] = vc_image_new(video.width, video.height, 3, 255);
		vc_rgb_to_hsv(img[1], img[2]);

		// Segmentação de uma imagem HSV
		img[3] = vc_image_new(video.width, video.height, 1, 255);
		vc_hsv_segmentation(img[2], img[3], 20, 50, 37, 100, 10, 100);

		// Dilatar e erodir a imagem para remover ruído
		// NÃO USAMOS PORQUE: não tem ganhos visíveis e aumenta o tempo de processamento
		// vc_binary_close(img[3], img[4], 3, 3);

		// // Pesquisa de blobs
		int nblobs;
		img[4] = vc_image_new(video.width, video.height, 1, 255);
		OVC *blobs = vc_binary_blob_labelling(img[3], img[4], &nblobs);
		if (blobs != NULL)
		{
			// Informação dos blobs
			vc_binary_blob_info(img[4], blobs, nblobs);

			// Percorrer os blobs
			for (int i = 0; i < nblobs; i++)
			{
				// Limpeza de blobs indesejados
				if (blobs[i].width > 100 && blobs[i].height < 100)
				{
					// Desenhar o centro de gravidade
					vc_draw_of_gravity(img[0], &blobs[i]);
					// Desenhar as bordas na imagem HSV
					vc_draw_border_box(img[0], &blobs[i]);
					// Identificar as cores presentes na borda do blob
					int resistencia = vc_filtro_resistencias(img[2], &blobs[i]);
					// Desenhar a resistência da resistência
					vc_draw_resistance_value(img[0], &blobs[i], resistencia);
				}
			}
		}

		// Copiar a imagem IVC para o frame
		memcpy(frame.data, img[0]->data, video.width * video.height * 3);

		// Libertar memória das imagens
		vc_image_free(img[0]);
		vc_image_free(img[1]);
		vc_image_free(img[2]);
		vc_image_free(img[3]);
		vc_image_free(img[4]);

		// Exibe o frame
		cv::imshow("VC - VIDEO", frame);

		// Sair da aplicação, se o utilizador premir a tecla 'q'
		key = cv::waitKey(1);
	}

	// Para o timer e exibe o tempo decorrido
	vc_timer();

	// Fecha a janela
	cv::destroyWindow("VC - VIDEO");

	// Fecha o ficheiro de v�deo
	capture.release();

	return 0;
}
