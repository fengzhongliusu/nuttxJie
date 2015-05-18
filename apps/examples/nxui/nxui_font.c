/*************************************************************************
    > File Name: nxui_font.c
    > Author: liujie
    > Mail: 1170381285@qq.com 
    > Created Time: Mon 04 May 2015 10:43:48 AM CST
 ************************************************************************/
#include <iconv.h>
#include <stdio.h>
//代码转换:从一种编码转为另一种编码
/*int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) {
		return -1;
		printf("open failed....\n");
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1){
	   	return -1;
	}
	iconv_close(cd);
	return 0;
}

//UNICODE码转为GB2312码
inline int u2g(const char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}*/
//代码转换:从一种编码转为另一种编码
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) {
		return -1;
		printf("open failed....\n");
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1){
		printf("converting error...\n");
		iconv_close(cd);
	   	return -1;
	}
	iconv_close(cd);
	return 0;
}

//UNICODE码转为GB2312码
int u2g(const char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	int ret =  code_convert("UTF-8","GB2312",inbuf,inlen,outbuf,outlen);
	printf("leave u2g\n");
	return ret;
}
