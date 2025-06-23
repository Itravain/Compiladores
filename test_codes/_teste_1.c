/*chamada de função sem parametro*/
int glob[5];
int var_glob;

int teste(){
    return 5+6;
}

void main()
{
    int x;
    x = teste();
    var_glob = x;
}
