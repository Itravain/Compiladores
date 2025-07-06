int vetor[5];
int tamanho;

int imprimir_vetor(int vetor_param[], int tamanho_param){
    output(tamanho);
    output(vetor_param[0]);
    output(vetor_param[1]);    
}

int main(){
    vetor[0] = 2;
    vetor[1] = 8;
    imprimir_vetor(vetor, 3);
}