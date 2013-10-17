int main()
{
	textInit();
	
	int someVar = 37;
	
	simplePrintf("someVar = %d; @ 0x%x", someVar, &someVar);
	return 0;
}
