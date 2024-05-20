//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG
#define MAX_VC(a, b) (a > b ? a : b)
#define MIN_VC(a, b) (a < b ? a : b)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct
{
	unsigned char *data;
	int width, height;
	int channels;	  // Bin�rio/Cinzentos=1; RGB=3
	int levels;		  // Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline; // width * channels
} IVC;

typedef struct
{
	int x, y, width, height;
	int area;
	int xc, yc;
	int perimeter;
	int label;

	unsigned char *mask;
	unsigned char *data;
	int channels;
	int levels;
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Filters
int vc_gray_lowpass_min_filter(IVC *src, IVC *dst, int kernel);
int vc_gray_lowpass_median_filter(IVC *src, IVC *dst, int kernel);
int vc_gray_gaussian_filter(IVC *src, IVC *dst);
int vc_gray_highpass_laplacian_filter(IVC *src, IVC *dst);

// Contornos
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);

// Histograma
int vc_color_equalize_image(IVC *src, IVC *dst);
int vc_gray_histogram_show(IVC *src, IVC *dst);
int vc_gray_equalize_image(IVC *src, IVC *dst);

// FUN��ES: LABELING
int vc_draw_of_gravity(IVC *srcdst, OVC *blob);
int vc_draw_border_box(IVC *srcdst, OVC *blob);
OVC *vc_blob_gray_coloring(IVC *src, IVC *dst, OVC *blobs, int nblobs);
OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

// FUN��ES: ERODE E DILATE
int vc_subtract(IVC *src, IVC *src2, IVC *dst);
int vc_grayscale_erode(IVC *src, IVC *dst, int kernel);
int vc_grayscale_dilate(IVC *src, IVC *dst, int kernel);
int vc_grayscale_open(IVC *src, IVC *dst, int kernel);
int vc_grayscale_close(IVC *src, IVC *dst, int kernel);
int vc_binary_open(IVC *src, IVC *dst, int kernel, int kernel2);
int vc_binary_close(IVC *src, IVC *dst, int kernel, int kernel2);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);

// FUNÇÕES: COMPARAÇÃO DE IMAGENS
int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, float k);
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int cMin);
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_global_mean(IVC *src, IVC *dst);
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold);
int vc_binary_area(IVC *src);
int vc_segmentation_area(IVC *src);

// FUN��ES: CONVERS�O DE IMAGENS
int vc_gray_to_rgb(IVC *src, IVC *dst);
int vc_hsv_to_rgb(IVC *src, IVC *dst);
void hsv_to_rgb(int h, int s, int v, unsigned char *r, unsigned char *g, unsigned char *b);
int vc_rgb_to_gray(IVC *src, IVC *dst);
int vc_rgb_to_hsv(IVC *src, IVC *dst);
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

// FUN��ES: EXTRAC��O DE CANAIS DE UMA IMAGEM RGB
int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_get_green_gray(IVC *srcdst);
int vc_rgb_get_blue_gray(IVC *srcdst);

// FUN��ES: INVERTER UMA IMAGEM
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
