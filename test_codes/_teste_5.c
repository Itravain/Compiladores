int tamanho;
int vetor[10];
int k;

void imprimir_vetor(int vetor_global_param[], int vetor_local_param[], int numero){
    k = 3;
    vetor[k] = 80;
    output(vetor[k]);
    vetor_global_param[k] = 18;
    output(vetor_global_param[k]);
    vetor_local_param[k] = 20;
    output(vetor_local_param[k]);
}

int main(){
    int i;
    int vetor_local[14];
    tamanho = 5;
    i = 7;
    vetor[i] = 8;
    output(vetor[i]);
    imprimir_vetor(vetor, vetor_local, 6);
}