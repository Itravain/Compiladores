int vet[3];

int minloc( int a[], int low, int high )
{	
	int i;
	int x;
	int k;
	k = low;
	x = a[low];
	i = low + 1;
	while (i < high){
		if (a[i] < x){
			x = a[i];
			k = i;
		}
		i = i + 1;
	}
	return k;
}

void sort( int a[], int low, int high)
{	int i; int k;
	i = low;
	while (i < high-1){
		int t;
		k = minloc(a,i,high);
		t = a[k];
		a[k] = a[i];
		a[i] = t;
		i = i + 1;
	}
}

void main(void)
{
	vet[0] = 15;
	vet[1] = 1;
	vet[2] = 5;
	sort(vet,0,3);
	output(vet[0]);
	output(vet[1]);
	output(vet[2]);
}