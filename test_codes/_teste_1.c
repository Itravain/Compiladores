/*chamada de função sem parametro*/
int var_glob;

int teste(int a, int b){
    int x;
    int z;
    a = b + 3;
    return a;
}

void main()
{
    int x;
    int u;
    x = teste(var_glob, u);
}
