/*chamada de função sem parametro*/
int var_glob;
int x[8];

int teste(){
    int a;
    int b;
    b = 1;
    a = b + 3;
    return a;
}

void main()
{
    x[3] = teste();
    output(x[3]); 
}
