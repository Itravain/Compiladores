void imprimir_vetor(int vetor_param[], int tamanho_param, int numero){
    int k;
    vetor_param[2] = numero;
    k = 3;
    vetor_param[k] = vetor_param[3];
    vetor_param[3] = vetor_param[k];

    output(tamanho_param);
    output(numero);
    output(vetor_param[0]);
    output(vetor_param[1]); 
    output(vetor_param[2]); 
}

int main(){
    int tamanho;
    int vetor[5];
    tamanho = 9;
    vetor[0] = 2;
    vetor[1] = 8;
    imprimir_vetor(vetor, tamanho, 3);
}