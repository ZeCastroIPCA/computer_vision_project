//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Filters
int vc_gray_lowpass_min_filter(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int i, j;
	int size = (kernel - 1) / 2;
	int sum;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_lowpass_min_filter() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (channels != 1)
	{
		printf("vc_gray_lowpass_min_filter() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}
	if (kernel < 3)
	{
		printf("vc_gray_lowpass_min_filter() - Kernel tem que ser pelo menos 3.\n");
		return 0;
	}
	if (kernel % 2 == 0)
	{
		printf("vc_gray_lowpass_min_filter() - Kernel tem que ser �mpar.\n");
		return 0;
	}

	// Limpa imagem destino
	memset(datadst, 0, bytesperline * height);

	// Faz a filtragem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			sum = 0;
			int count = 0;

			// Somar todos os pixeis da vizinhan�a
			for (j = -size; j <= size; j++)
			{
				for (i = -size; i <= size; i++)
				{
					// Verifica se o pixel est� dentro dos limites da imagem
					if ((y + j >= 0) && (y + j < height) && (x + i >= 0) && (x + i < width))
					{
						pos = (y + j) * bytesperline + (x + i) * channels;
						sum += datasrc[pos];
						count++;
					}
				}
			}

			// Faz a m�dia e atribui ao pixel
			pos = y * bytesperline + x * channels;
			datadst[pos] = (unsigned char)(sum / count);
		}
	}

	return 1;
}

int vc_gray_lowpass_median_filter(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int i, j;
	int size = (kernel - 1) / 2;
	int sum;
	unsigned char *window;
	int window_size = kernel * kernel;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_median_filter() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (channels != 1)
	{
		printf("vc_gray_median_filter() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}
	if (kernel < 3)
	{
		printf("vc_gray_median_filter() - Kernel tem que ser pelo menos 3.\n");
		return 0;
	}
	if (kernel % 2 == 0)
	{
		printf("vc_gray_median_filter() - Kernel tem que ser �mpar.\n");
		return 0;
	}

	// Limpa imagem destino
	memset(datadst, 0, bytesperline * height);

	// Alocar mem�ria para a janela
	window = (unsigned char *)malloc(window_size * sizeof(unsigned char));
	if (window == NULL)
	{
		printf("vc_gray_median_filter() - N�o foi poss�vel alocar mem�ria para a janela.\n");
		return 0;
	}

	// Faz a filtragem
	for (y = size; y < height - size; y++)
	{
		for (x = size; x < width - size; x++)
		{
			pos = y * bytesperline + x * channels;

			// Copiar valores da vizinhan�a para a janela
			int k = 0;
			for (j = -size; j <= size; j++)
			{
				for (i = -size; i <= size; i++)
				{
					pos = (y + j) * bytesperline + (x + i) * channels;
					window[k] = datasrc[pos];
					k++;
				}
			}

			// Ordenar a janela
			for (i = 0; i < window_size; i++)
			{
				for (j = i + 1; j < window_size; j++)
				{
					if (window[i] > window[j])
					{
						unsigned char temp = window[i];
						window[i] = window[j];
						window[j] = temp;
					}
				}

				// Atribuir o valor mediano ao pixel
				pos = y * bytesperline + x * channels;
				datadst[pos] = window[window_size / 2];
			}
		}
	}

	// Libertar mem�ria da janela
	free(window);

	return 1;
}

int vc_gray_gaussian_filter(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int i, j;
	int size = (5 - 1) / 2;
	int grad;
	float mask[5][5] = {{1, 4, 7, 4, 1},
						{4, 16, 26, 16, 4},
						{7, 26, 41, 26, 7},
						{4, 16, 26, 16, 4},
						{1, 4, 7, 4, 1}};

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_gaussian_filter() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (channels != 1)
	{
		printf("vc_gray_gaussian_filter() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}

	// Normaliza��o factor de escala
	float norm_factor = 1.0f / 273.0f;

	// Convolu��o para x e y
	for (y = size; y < height - size; y++)
	{
		for (x = size; x < width - size; x++)
		{
			pos = y * bytesperline + x * channels;
			grad = 0;

			// M�scara de convolu��o: Gaussian
			for (j = -size; j <= size; j++)
			{
				for (i = -size; i <= size; i++)
				{
					pos = (y + j) * bytesperline + (x + i) * channels;
					grad += (int)(datasrc[pos]) * mask[j + 2][i + 2];
				}
			}

			// Armazena o valor na imagem destino
			pos = y * bytesperline + x * channels;
			datadst[pos] = (unsigned char)(grad * norm_factor);
		}
	}

	return 1;
}

int vc_gray_highpass_laplacian_filter(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int i, j;
	int size = 3;
	int mask[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
	int mask2[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	int mask3[3][3] = {{1, -2, 1}, {-2, 4, -2}, {1, -2, 1}};
	float norm_factor = 1.0f / 6.0f;
	float norm_factor2 = 1.0f / 9.0f;
	float norm_factor3 = 1.0f / 3.0f;
	float grad;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_highpass_laplacian_filter() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (channels != 1)
	{
		printf("vc_gray_highpass_laplacian_filter() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}

	// convolu��o para x e y
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			grad = 0;

			// M�scara de convolu��o: Laplacian
			for (j = 0; j < size; j++)
			{
				for (i = 0; i < size; i++)
				{
					pos = (y + j - 2) * bytesperline + (x + i - 2) * channels;
					grad += (int)(datasrc[pos]) * mask[j][i];
				}
			}

			grad = fabs(grad);

			// Armazena o valor na imagem destino
			pos = y * bytesperline + x * channels;
			datadst[pos] = (unsigned char)(grad * norm_factor);
		}
	}

	return 1;
}

// Contornos
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int size = 3;
	float grad, sumX, sumY;
	float maskX[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
	float maskY[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_edge_prewitt() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (channels != 1)
	{
		printf("vc_gray_edge_prewitt() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}

	// Limiar de detec��o de arestas
	if ((th < 0) || (th > 1))
	{
		printf("vc_gray_edge_prewitt() - O limiar de detec��o de arestas tem que estar entre 0 e 1.\n");
		return 0;
	}

	// Limpa imagem destino
	memset(datadst, 0, bytesperline * height);

	// C�lculo da magnitude do gradiente
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;
			sumX = 0;
			sumY = 0;

			// M�scara de convolu��o: Prewitt
			for (int l = 0; l < size; l++)
			{
				for (int m = 0; m < size; m++)
				{
					pos = (y + l - 1) * bytesperline + (x + m - 1) * channels;
					sumX += (float)(datasrc[pos]) * maskX[l][m];
					sumY += (float)(datasrc[pos]) * maskY[l][m];
				}
			}

			// Magnitude do gradiente
			grad = 1 * sqrt(2) * sqrt(sumX * sumX + sumY * sumY);

			// Armazena o valor na imagem destino
			datadst[y * bytesperline + x * channels] = (unsigned char)grad;
		}
	}

	// Calcular histograma
	int histogram[256] = {0};
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			histogram[datadst[pos]]++;
		}
	}

	// Calculate threshold value based on percentage
	int threshold = 0;
	int sum = 0;
	int total_pixels = width * height;
	float target_sum = total_pixels * th; // Convertendo o percentual em número de pixels

	// Calculating threshold value based on histogram accumulation
	for (int i = 0; i < 256; i++)
	{
		sum += histogram[i];
		if (sum >= target_sum)
		{
			threshold = i;
			break;
		}
	}

	// Binarization of the image based on threshold
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			if (datadst[pos] > threshold)
				datadst[pos] = 255; // Pintar pixel de branco se a magnitude for maior que o limite
			else
				datadst[pos] = 0; // Pintar pixel de preto caso contrário
		}
	}

	return 1;
}

// Histogram
int vc_color_equalize_image(IVC *src, IVC *dst)
{
	// histogram para a saturação e o valor da imagem
	unsigned char *datasrc = src->data;
	unsigned char *datadst = dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int histogramsaturation[256] = {0}; // Initialize histogram arrays
	int histogramvalue[256] = {0};

	// Check if destination image pointer is valid
	if (datadst == NULL)
	{
		printf("vc_color_equalize_image() - Invalid destination image pointer.\n");
		return 0;
	}

	// Initialize destination image to zeros
	memset(datadst, 0, bytesperline * height);

	// Calculate histograms for saturation and value
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			histogramsaturation[datasrc[pos + 1]]++;
			histogramvalue[datasrc[pos + 2]]++;
		}
	}

	// Calculate cumulative distribution functions (CDF) for saturation and value
	int cdfsaturation[256];
	int cdfvalue[256];
	cdfsaturation[0] = histogramsaturation[0];
	cdfvalue[0] = histogramvalue[0];
	for (i = 1; i < 256; i++)
	{
		cdfsaturation[i] = cdfsaturation[i - 1] + histogramsaturation[i];
		cdfvalue[i] = cdfvalue[i - 1] + histogramvalue[i];
	}

	// Find the minimum non-zero CDF value for saturation and value
	int cdf_min_saturation = 0;
	int cdf_min_value = 0;
	for (i = 0; i < 256; i++)
	{
		if (cdfsaturation[i] != 0)
		{
			cdf_min_saturation = cdfsaturation[i];
			break;
		}
	}
	for (i = 0; i < 256; i++)
	{
		if (cdfvalue[i] != 0)
		{
			cdf_min_value = cdfvalue[i];
			break;
		}
	}

	// Normalize the histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			// Ensure cdf_min is not zero to avoid division by zero
			datadst[pos] = datasrc[pos];
			if (cdfsaturation[255] - cdf_min_saturation != 0)
			{
				datadst[pos + 1] = (unsigned char)(((cdfsaturation[datasrc[pos + 1]] - cdf_min_saturation) / (float)(cdfsaturation[255] - cdf_min_saturation)) * 255);
			}
			if (cdfvalue[255] - cdf_min_value != 0)
			{
				datadst[pos + 2] = (unsigned char)(((cdfvalue[datasrc[pos + 2]] - cdf_min_value) / (float)(cdfvalue[255] - cdf_min_value)) * 255);
			}
		}
	}

	return 1;
}

int vc_gray_equalize_image(IVC *src, IVC *dst)
{
	int x, y;
	long int pos;
	unsigned int histogram[256];
	unsigned char *data = (unsigned char *)src->data;
	unsigned char *data_dst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int levels = src->levels;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_equalize_image() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (src->channels != 1)
	{
		printf("vc_gray_equalize_image() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}

	// Contar quantos pixeis existem para cada valor de pixel (0-255)
	for (int i = 0; i < 256; i++)
		histogram[i] = 0;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * src->channels;
			histogram[data[pos]]++;
		}
	}

	// Calcular cdf (cumulative distribution function)
	unsigned int cdf[256];
	cdf[0] = histogram[0];
	for (int i = 1; i < 256; i++)
		cdf[i] = cdf[i - 1] + histogram[i];

	// Normalizar o histograma para o intervalo [0,255]
	unsigned int cdf_min = cdf[0];
	int total_pixels = width * height;
	for (int i = 0; i < 256; i++)
	{
		cdf[i] = ((cdf[i] - cdf_min) * (levels - 1)) / (total_pixels - cdf_min);
	}

	// Aplicar a transforma��o
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * src->channels;
			data_dst[pos] = cdf[data[pos]];
		}
	}

	return 1;
}

int vc_gray_histogram_show(IVC *src, IVC *dst)
{
	int x, y;
	long int pos;
	unsigned int histogram[256];
	unsigned char *data = (unsigned char *)src->data;
	unsigned char *data_dst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("vc_gray_histogram_show() - Erro nos parametros de entrada.\n");
		return 0;
	}
	if (src->channels != 1)
	{
		printf("vc_gray_histogram_show() - A imagem de entrada n�o � de cinzentos.\n");
		return 0;
	}

	// Contar quantos pixeis existem para cada valor de pixel (0-255)
	for (int i = 0; i < 256; i++)
		histogram[i] = 0;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * src->channels;
			histogram[data[pos]]++;
		}
	}

	// Imprimir para a consola
	printf("Histograma:\n");
	printf("----------\n");
	printf("Valor de Pixel | Nr. de Pixeis \n");
	printf("---------------|---------------\n");
	for (int i = 0; i < 256; i++)
	{
		printf("%14d | %13d \n", i, histogram[i]);
	}

	// Normalizar o histograma para o intervalo [0,255]
	int maxvalue = 0;
	for (int i = 0; i < 256; i++)
	{
		if (histogram[i] > maxvalue)
			maxvalue = histogram[i];
	}

	// Offset para centrar o histograma
	int offset = 0;
	if (dst->width > 256)
		offset = (dst->width - 256) / 2;

	// Desenhar o histograma numa imagem com tamanho variável (ajusta o histograma ao tamanho da imagem)
	for (int i = 0; i < 256; i++)
	{
		int value = (histogram[i] * dst->height) / maxvalue;
		for (int j = 0; j < dst->height; j++)
		{
			printf("value: %d\n", value);
			if (j < value)
				data_dst[(dst->height - 1 - j) * dst->bytesperline + (offset + i) * dst->channels] = 255;
			else
				data_dst[(dst->height - 1 - j) * dst->bytesperline + (offset + i) * dst->channels] = 0;
		}
	}

	return 1;
}

// Labeling
int vc_draw_of_gravity(IVC *srcdst, OVC *blob)
{
	int c, x, y;

	// draw vertical line
	for (y = blob->yc - 2; y < blob->yc + 3; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->xc * srcdst->channels + c] = 0;
		}
	}

	// draw horizontal line
	for (x = blob->xc - 2; x < blob->xc + 3; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->yc * srcdst->bytesperline + x * srcdst->channels + c] = 0;
		}
	}

	return 1;
}

int vc_draw_border_box(IVC *srcdst, OVC *blob)
{
	int c, x, y;

	for (y = blob->y; y < blob->y + blob->height; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->x * srcdst->channels] = 255;
			srcdst->data[y * srcdst->bytesperline + (blob->x + blob->width - 1) * srcdst->channels] = 255;
		}
	}

	for (x = blob->x; x < blob->x + blob->width; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->y * srcdst->bytesperline + x * srcdst->channels] = 255;
			srcdst->data[(blob->y + blob->height - 1) * srcdst->bytesperline + x * srcdst->channels] = 255;
		}
	}

	return 1;
}

OVC *vc_blob_gray_coloring(IVC *src, IVC *dst, OVC *blobs, int nblobs)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;

	// Verificacao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return NULL;
	if (channels != 1)
		return NULL;

	// Copia dados da imagem binaria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Pinta os blobs
	for (i = 0; i < nblobs; i++)
	{
		unsigned char color = 120 + (i * 80) % 255;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				pos = y * bytesperline + x * channels;
				if (datadst[pos] == blobs[i].label)
				{
					datadst[pos] = color;
				}
			}
		}
	}

	return blobs;
}

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = {0};
	int labelarea[256] = {0};
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser retornado desta funo.

	// Verificao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("vc_binary_blob_labelling() --> Input image and output image must have the same dimensions!\n");
		return NULL;
	}
	if (channels != 1)
		return NULL;

	// Copia dados da imagem binria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixis de plano de fundo devem obrigatriamente ter valor 0
	// Todos os pixis de primeiro plano devem obrigatriamente ter valor 255
	// Sero atribudas etiquetas no intervalo [1,254]
	// Este algoritmo est assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0)
			datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}

	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels;		// B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels;		// D
			posX = y * bytesperline + x * channels;				// X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est marcado
					if (datadst[posA] != 0)
						num = labeltable[datadst[posA]];
					// Se B est marcado, e  menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num))
						num = labeltable[datadst[posB]];
					// Se C est marcado, e  menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num))
						num = labeltable[datadst[posC]];
					// Se D est marcado, e  menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num))
						num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// printf("\nMax Label = %d\n", label);

	// Contagem do nmero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b])
				labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que no hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++;						  // Conta etiquetas
		}
	}

	// Se no h blobs
	if (*nlabels == 0)
	{
		printf("vc_binary_blob_labelling() --> No objects found!\n");
		return NULL;
	}

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++)
			blobs[a].label = labeltable[a];
	}
	else
	{
		printf("vc_binary_blob_labelling() --> Memory Allocation Error!\n");
		return NULL;
	}

	return blobs;
}

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// Conta rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;

					// Permetro
					// Se pelo menos um dos quatro vizinhos no pertence ao mesmo label, ento  um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		// blobs[i].xc = (xmax - xmin) / 2;
		// blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX_VC(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX_VC(blobs[i].area, 1);
	}

	return 1;
}

int vc_subtract(IVC *src, IVC *src2, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int bytesperline_src = width * channels_src;
	int bytesperline_dst = width * channels_dst;
	int x, y;
	long int pos_src, pos_dst;

	// Verificação de erros
	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->channels != 1 || dst->channels != 1)
		return 0;

	// Percorrer os pixeis e subtrair o valor do pixel da imagem de destino
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			datadst[pos_dst] = datasrc[pos_src] - src2->data[pos_src];
		}
	}
	return 1;
}

int vc_grayscale_open(IVC *src, IVC *dst, int kernel)
{
	IVC *temp = vc_image_new(src->width, src->height, 1, 255);
	if (temp == NULL)
	{
		printf("vc_grayscale_open():\n\tError creating temporary image!\n");
		return 0;
	}

	if (vc_grayscale_erode(src, temp, kernel) == 0)
	{
		printf("vc_grayscale_open():\n\tError in erosion operation!\n");
		vc_image_free(temp);
		return 0;
	}

	if (vc_grayscale_dilate(temp, dst, kernel) == 0)
	{
		printf("vc_grayscale_open():\n\tError in dilation operation!\n");
		vc_image_free(temp);
		return 0;
	}

	vc_image_free(temp);
	return 1;
}

int vc_grayscale_close(IVC *src, IVC *dst, int kernel)
{
	IVC *temp = vc_image_new(src->width, src->height, 1, 255);
	if (temp == NULL)
	{
		printf("vc_grayscale_open():\n\tError creating temporary image!\n");
		return 0;
	}

	if (vc_grayscale_dilate(src, temp, kernel) == 0)
	{
		printf("vc_grayscale_open():\n\tError in dilation operation!\n");
		vc_image_free(temp);
		return 0;
	}

	if (vc_grayscale_erode(temp, dst, kernel) == 0)
	{
		printf("vc_grayscale_open():\n\tError in erosion operation!\n");
		vc_image_free(temp);
		return 0;
	}

	vc_image_free(temp);
	return 1;
}

int vc_grayscale_erode(IVC *src, IVC *dst, int kernel)
{
	int width, height, channels, bytesperline;
	long int pos, posk;
	unsigned char *datasrc, *datadst;
	int offset = (kernel - 1) / 2;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL))
		return 0;
	if (src->channels != 1)
		return 0;
	if (dst->channels != 1)
		return 0;

	// Atribuição das variáveis
	width = src->width;
	height = src->height;
	channels = src->channels;
	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;
	bytesperline = width * channels;

	// percorrer os pixeis e encontrar o mínimo valor na vizinhança
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			unsigned char min_pixel = 255; // Initialize to maximum grayscale value

			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					int new_y = y + i;
					int new_x = x + j;

					// Check boundaries
					if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
					{
						posk = new_y * bytesperline + new_x * channels;
						if (datasrc[posk] < min_pixel) // Find minimum pixel value in the neighborhood
						{
							min_pixel = datasrc[posk];
						}
					}
				}
			}
			datadst[pos] = min_pixel; // Assign the minimum value to the central pixel
		}
	}
	return 1;
}

int vc_grayscale_dilate(IVC *src, IVC *dst, int kernel)
{
	int width, height, channels, bytesperline;
	long int pos, posk;
	unsigned char *datasrc, *datadst;
	int offset = (kernel - 1) / 2;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL))
		return 0;
	if (src->channels != 1)
		return 0;
	if (dst->channels != 1)
		return 0;

	// Atribuição das variáveis
	width = src->width;
	height = src->height;
	channels = src->channels;
	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;
	bytesperline = width * channels;

	// percorrer os pixeis e se encontrar um pixel branco, preencher a vizinhança com branco
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			unsigned char max_pixel = 0; // Initialize to minimum value

			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					int new_y = y + i;
					int new_x = x + j;

					// Check boundaries
					if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
					{
						posk = new_y * bytesperline + new_x * channels;
						unsigned char src_pixel = datasrc[posk];
						if (src_pixel > max_pixel)
						{
							max_pixel = src_pixel;
						}
					}
				}
			}
			datadst[pos] = max_pixel;
		}
	}
	return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel, int kernel2)
{
	IVC *aux = vc_image_new(src->width, src->height, 1, 255);
	vc_binary_erode(src, aux, kernel);
	vc_binary_dilate(aux, dst, kernel2);
	vc_image_free(aux);

	return 1;
}

int vc_binary_close(IVC *src, IVC *dst, int kernel, int kernel2)
{
	IVC *aux = vc_image_new(src->width, src->height, 1, 255);
	vc_binary_dilate(src, aux, kernel);
	vc_binary_erode(aux, dst, kernel2);
	vc_image_free(aux);

	return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
	int width, height, channels, bytesperline;
	long int pos, posk;
	unsigned char *datasrc, *datadst;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL))
		return 0;
	if (src->channels != 1)
		return 0;
	if (dst->channels != 1)
		return 0;

	// Atribuição das variáveis
	width = src->width;
	height = src->height;
	channels = src->channels;
	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;
	bytesperline = width * channels;
	int offset = (kernel - 1) / 2; // Corrigindo a fórmula para calcular o offset

	// Percorrer a imagem e aplicar a operação de erosão
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			// Verificar se o pixel central é branco
			if (datasrc[pos] == 255)
			{
				int erosion = 1; // Variável para indicar se ocorreu erosão
				// Percorrer a vizinhança
				for (int i = -offset; i <= offset; i++)
				{
					for (int j = -offset; j <= offset; j++)
					{
						int new_y = y + i;
						int new_x = x + j;
						if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
						{
							posk = new_y * bytesperline + new_x * channels;
							// Se algum pixel na vizinhança for preto, realizar a erosão
							if (datasrc[posk] == 0)
							{
								erosion = 0; // Não realizar erosão neste caso
								break;		 // Sair do loop interno
							}
						}
					}
					if (erosion == 0)
						break; // Sair do loop externo
				}
				// Se nenhum pixel na vizinhança for preto, realizar a erosão
				if (erosion == 1)
				{
					datadst[pos] = 255; // Preencher o pixel central com branco
				}
				else
				{
					datadst[pos] = 0; // Caso contrário, preencher o pixel central com preto
				}
			}
			else
			{
				datadst[pos] = 0; // Se o pixel central não for branco, preencher com preto
			}
		}
	}

	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
	int width, height, channels, bytesperline;
	long int pos, posk;
	unsigned char *datasrc, *datadst;
	int offset = (kernel - 1) / 2;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL))
		return 0;
	if (src->channels != 1)
		return 0;
	if (dst->channels != 1)
		return 0;

	// Atribuição das variáveis
	width = src->width;
	height = src->height;
	channels = src->channels;
	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;
	bytesperline = width * channels;

	// percorrer os pixeis e se encontrar um pixel branco, preencher a vizinhança com branco
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			unsigned char pixel = 0; // Initialize to black

			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					int new_y = y + i;
					int new_x = x + j;

					// Check boundaries
					if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
					{
						posk = new_y * bytesperline + new_x * channels;
						pixel |= datasrc[posk]; // Perform OR operation with neighboring pixel
					}
				}
			}
			datadst[pos] = pixel;
		}
	}
	return 1;
}

int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, float k)
{
	int x, y;
	int width, height, channels, bytesperline;
	long int pos, pos_dst, posk;
	unsigned char *datasrc, *datadst, mean, sdeviation;
	float offset, threshold;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->channels <= 0) || (src->levels <= 0) || (src->data == NULL) || (dst->data == NULL))
		return 0;

	// Atribuição das variáveis
	width = src->width;
	height = src->height;
	channels = src->channels;
	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;
	offset = (kernel - 1) / 2;
	bytesperline = width * channels;

	// Percorrer todos os pixels para fazer a média e o desvio padrão
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int sum = 0;
			int count = 0;
			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					int new_y = y + i;
					int new_x = x + j;
					if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
					{
						posk = (new_y * width + new_x) * channels; // Ajuste da fórmula de posição
						sum += datasrc[posk];
						count++;
					}
				}
			}
			mean = sum / count;
			sdeviation = 0.0f;
			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					int new_y = y + i;
					int new_x = x + j;
					if (new_y >= 0 && new_y < height && new_x >= 0 && new_x < width)
					{
						posk = (new_y * width + new_x) * channels; // Ajuste da fórmula de posição
						sdeviation += (datasrc[posk] - mean) * (datasrc[posk] - mean);
					}
				}
			}
			sdeviation = sqrtf(sdeviation / (count > 0 ? count : 1)); // Evitar divisão por zero
			threshold = mean + k * sdeviation;
			pos_dst = (y * width + x) * channels; // Ajuste da fórmula de posição

			if (datasrc[pos_dst] > threshold)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;
		}
	}
	return 1; // Retornar 1 para indicar sucesso
}

int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int cMin)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_gray_to_binary_midpoint():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1)
	{
		printf("Error -> vc_gray_to_binary_midpoint():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	int sum = 0;
	int count = 0;
	int min = 255;
	int max = 0;
	unsigned char mean = 0;

	// calculate offset
	int offset = (kernel - 1) / 2;

	// use cMin to calculate the threshold using bernsen
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			min = 255;
			max = 0;

			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					if ((y + i >= 0) && (y + i < height) && (x + j >= 0) && (x + j < width))
					{
						pos_src = (y + i) * bytesperline_src + (x + j) * channels_src;

						if (datasrc[pos_src] < min)
						{
							min = datasrc[pos_src];
						}
						if (datasrc[pos_src] > max)
						{
							max = datasrc[pos_src];
						}
					}
				}
			}

			mean = (max - min) <= cMin ? src->levels / 2 : ((max + min) / 2);

			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_dst] > mean)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_gray_to_binary_midpoint():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1)
	{
		printf("Error -> vc_gray_to_binary_midpoint():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	int sum = 0;
	int count = 0;
	int min = 255;
	int max = 0;
	unsigned char mean = 0;

	// calculate offset
	int offset = (kernel - 1) / 2;

	// calculate the mean of the image
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			min = 255;
			max = 0;

			for (int i = -offset; i <= offset; i++)
			{
				for (int j = -offset; j <= offset; j++)
				{
					if ((y + i >= 0) && (y + i < height) && (x + j >= 0) && (x + j < width))
					{
						pos_src = (y + i) * bytesperline_src + (x + j) * channels_src;

						if (datasrc[pos_src] < min)
						{
							min = datasrc[pos_src];
						}
						if (datasrc[pos_src] > max)
						{
							max = datasrc[pos_src];
						}
					}
				}
			}

			mean = (min + max) / 2;

			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_dst] > mean)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

// thresholding gray image
int vc_gray_to_binary_global_mean(IVC *src, IVC *dst)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_gray_to_binary():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1)
	{
		printf("Error -> vc_gray_to_binary():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	// calculate the mean of the image
	int mean = 0;
	int count = 0;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * width + x;
			pos_dst = y * width + x;

			count += datasrc[pos_src];
		}
	}
	mean = count / (width * height);

	// create the image
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * width + x;
			pos_dst = y * width + x;

			if (datasrc[pos_src] > mean)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

// thresholding gray image
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_gray_to_binary():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1)
	{
		printf("Error -> vc_gray_to_binary():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	// thresholding
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * width + x;
			pos_dst = y * width + x;

			if (datasrc[pos_src] > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

// get area of the brain segmentation
int vc_segmentation_area(IVC *src)
{
	if (src == NULL || src->data == NULL)
	{
		printf("Error -> vc_binary_area():\n\tImage is empty!\n");
		return 0;
	}
	if (src->channels != 1)
	{
		printf("Error -> vc_binary_area():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int x, y;
	long int pos;
	int count = 0;

	// search every pixel of the image for the value of the pixel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * width + x;

			if (data[pos] != 0)
			{
				count++;
			}
		}
	}

	return count;
}

// get area of the brain
int vc_binary_area(IVC *src)
{
	if (src == NULL || src->data == NULL)
	{
		printf("Error -> vc_binary_area():\n\tImage is empty!\n");
		return 0;
	}
	if (src->channels != 1)
	{
		printf("Error -> vc_binary_area():\n\tImage is not grayscale!\n");
		return 0;
	}

	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int x, y;
	long int pos;
	int count = 0;

	// search every pixel of the image for the value of the pixel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * width + x;

			if (data[pos] < 11)
			{
				count++;
			}
		}
	}

	return count;
}

// Converter uma imagem cinzenta para uma imagem RGB
int vc_gray_to_rgb(IVC *src, IVC *dst)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 3)
		return 0;

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	// definir array para os canais
	int r[256], g[256], b[256];

	for (int i = 0; i < 256; i++)
	{
		// vermelho: 0 - 128 = 0; 129 - 192 = crescente; 193 - 255 = 255
		// verde: 0 - 64 = crescente; 65 - 192 = 255; 193 - 255 = decrescente
		// azul: 0 - 64 = 255; 65 - 128 = decrescente; 129 - 255 = 0
		if (i <= 64)
		{
			r[i] = 0;
			g[i] = (int)((float)i / 64.0f * 255.0f);
			b[i] = 255;
		}
		else if (i <= 128)
		{
			r[i] = 0;
			g[i] = 255;
			b[i] = (int)((float)(128 - i) / 64.0f * 255.0f);
		}
		else if (i <= 192)
		{
			r[i] = (int)((float)(i - 128) / 64.0f * 255.0f);
			g[i] = 255;
			b[i] = 0;
		}
		else if (i <= 255)
		{
			r[i] = 255;
			g[i] = (int)((float)(255 - i) / 63.0f * 255.0f);
			b[i] = 0;
		}
	}

	// converter imagem cinzenta para imagem RGB utilizando os arrays de canais r, g e b definidos anteriormente
	// cada pixel da imagem cinzenta é convertido para um pixel RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * width + x;
			pos_dst = y * width * 3 + x * 3;

			datadst[pos_dst] = r[datasrc[pos_src]];
			datadst[pos_dst + 1] = g[datasrc[pos_src]];
			datadst[pos_dst + 2] = b[datasrc[pos_src]];
		}
	}

	return 1;
}

// Segmentar uma imagem HSV
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_hsv_segmentation():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 3 || dst->channels != 1)
	{
		printf("Error -> vc_hsv_segmentation():\n\tImage is not RGB!\n");
		return 0;
	}

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	int h, s, v;

	int size = width * height * channels_src;

	for (int i = 0; i < size; i = i + channels_src)
	{
		h = (int)((float)datasrc[i] / 255.0f * 360.0f);
		s = (int)((float)datasrc[i + 1] / 255.0f * 100.0f);
		v = (int)((float)datasrc[i + 2] / 255.0f * 100.0f);

		if ((h >= hmin && h <= hmax) && (s >= smin && s <= smax) && (v >= vmin && v <= vmax))
		{
			datadst[i / channels_src] = 255;
		}
		else
		{
			datadst[i / channels_src] = 0;
		}
	}

	return 1;
}

// Transformar uma imagem HSV para uma imagem RGB
int vc_hsv_to_rgb(IVC *src, IVC *dst)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
	{
		printf("Error -> vc_hsv_to_rgb():\n\tImage is empty!\n");
		return 0;
	}
	if (src->width != dst->width || src->height != dst->height || src->channels != 3 || dst->channels != 3)
	{
		printf("Error -> vc_hsv_to_rgb():\n\tImage is not HSV!\n");
		return 0;
	}

	unsigned char *datasrc = src->data;
	unsigned char *datadst = dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int h, s, v;
	unsigned char r, g, b;
	long int pos;

	// Check if source and destination image pointers are valid
	if (datasrc == NULL || datadst == NULL)
	{
		printf("Error -> vc_hsv_to_rgb():\n\tImage is empty!\n");
		return 0;
	}

	// Convert each pixel from HSV to RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			h = (float)datasrc[pos];			  // Hue
			s = (float)datasrc[pos + 1] / 255.0f; // Saturation
			v = (float)datasrc[pos + 2] / 255.0f; // Value

			// Convert HSV to RGB
			hsv_to_rgb(h, s, v, &r, &g, &b);

			// Write RGB values to destination image
			datadst[pos] = r;
			datadst[pos + 1] = g;
			datadst[pos + 2] = b;
		}
	}
	return 1;
}

void hsv_to_rgb(int h, int s, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
	int i;
	int f, p, q, t;

	if (s == 0)
	{
		// Achromatic (grey)
		*r = *g = *b = (unsigned char)(v * 255);
		return;
	}

	h /= 60; // sector 0 to 5
	i = (int)floor(h);
	f = h - i; // factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));

	switch (i)
	{
	case 0:
		*r = (unsigned char)(v * 255);
		*g = (unsigned char)(t * 255);
		*b = (unsigned char)(p * 255);
		break;
	case 1:
		*r = (unsigned char)(q * 255);
		*g = (unsigned char)(v * 255);
		*b = (unsigned char)(p * 255);
		break;
	case 2:
		*r = (unsigned char)(p * 255);
		*g = (unsigned char)(v * 255);
		*b = (unsigned char)(t * 255);
		break;
	case 3:
		*r = (unsigned char)(p * 255);
		*g = (unsigned char)(q * 255);
		*b = (unsigned char)(v * 255);
		break;
	case 4:
		*r = (unsigned char)(t * 255);
		*g = (unsigned char)(p * 255);
		*b = (unsigned char)(v * 255);
		break;
	default:
		*r = (unsigned char)(v * 255);
		*g = (unsigned char)(p * 255);
		*b = (unsigned char)(q * 255);
		break;
	}
}

// Transformar uma imagem RGB para uma imagem HSV
int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height || src->channels != 3 || dst->channels != 3)
		return 0;

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = dst->width;
	int height = dst->height;
	int bytesperline = dst->bytesperline;
	int channels = dst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL))
		return 0;
	if (channels != 3)
		return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		r = (float)datasrc[i];
		g = (float)datasrc[i + 1];
		b = (float)datasrc[i + 2];

		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores à imagem destino
		datadst[i] = (unsigned char)((hue / 360.0f) * 255.0f);
		datadst[i + 1] = (unsigned char)(saturation);
		datadst[i + 2] = (unsigned char)(value);
	}

	return 1;
}

// Transformar uma imagem RGB para uma imagem cinzenta
int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height || src->channels != 3 || dst->channels != 1)
		return 0;

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int bytesperline_src = width * channels_src;
	int bytesperline_dst = width * channels_dst;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}

	return 1;
}

// Extrair o canal vermelho de uma imagem RGB
int vc_rgb_get_red_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;
	// Extrai a componente Red
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos + 1] = data[pos]; // Green
			data[pos + 2] = data[pos]; // Blue
		}
	}
	return 1;
};

// Extrair o canal verde de uma imagem RGB
int vc_rgb_get_green_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;
	// Extrai a componente Red
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos + 1];	   // Red
			data[pos + 2] = data[pos + 1]; // Blue
		}
	}
	return 1;
};

// Extrair o canal azul de uma imagem RGB
int vc_rgb_get_blue_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;
	// Extrai a componente Red
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos + 2];	   // Red
			data[pos + 1] = data[pos + 2]; // Green
		}
	}
	return 1;
};

// Inverter uma imagem a cores
int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;
	// Inverte a imagem RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}
	return 1;
}

// Inverter uma imagem cinzenta
int vc_gray_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 1)
		return 0;
	// Inverte a imagem Gray
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
		}
	}
	return 1;
}

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				// datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MAX_VC(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MAX_VC(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}