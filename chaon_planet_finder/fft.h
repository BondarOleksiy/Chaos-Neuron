#define FFT_FORWARD 1

int FFT( int dir,int m,double *x,double *y);
void do_hamming_window(double *array, int array_size);
void do_blackman_window(double *array, int array_size);