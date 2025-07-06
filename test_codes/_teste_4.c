int tamanho;

int imprimir_vetor(int vetor_param[], int tamanho_param, int numero){
    output(tamanho);
    output(vetor_param[0]);
    output(vetor_param[1]);    
}

int main(){
    int x;
    int vetor[5];
    vetor[0] = 2;
    vetor[1] = 8;
    x = vetor[0];
    imprimir_vetor(vetor, tamanho, 3);
}