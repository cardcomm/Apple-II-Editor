
/*
	func. to wait for a	keypress
	returns	keypress as	an int.
*/
waitkey()	{
	char	*keyboard =	0xc000;
	char	*strobe	= 0xc010;
	int		c;

	while(*keyboard<128);
	c =	*keyboard-128;
	*strobe	= 0;
	return(c);
}
