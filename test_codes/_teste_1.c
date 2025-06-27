/*chamada de função sem parametro*/
int var_glob;

int teste(int a){
    a = a + 3;
    return a;
}

void main()
{
    int x;
    x = teste(var_glob);
}
