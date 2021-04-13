#ifndef _GT60L16M2K4_H
#define _GT60L16M2K4_H


//�ͻ��Լ����죬�� addr��ַ�л�ȡ len ���ȵ����ݣ������p������
//extern unsigned char r_dat_bat(unsigned long  address,unsigned int byte_long,unsigned char *p_arr);//�ͻ����캯��
extern unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr);

#define ASCII_16X16_T      1
#define ASCII_16X16_A      2
#define ASCII_24X24_T      3
#define ASCII_24X24_A      4 
#define ASCII_32X32_T      5
#define ASCII_32X32_A      6

//����ASCII
unsigned char ASCII_GetData(unsigned char ASCIICode,unsigned long type,unsigned char*DZ_Data);//��ȡ���ȿ��ASCII�ַ�


#define cr_zf_8X16         1	//������������
#define cr_zf_16X16_T      2
#define cr_zf_16X16_A      3
#define cr_zf_8X16_F      4		//������������

#define cr_zf_24X24_T      5
#define cr_zf_24X24_A      6
#define cr_zf_16X24_F      7	//������������
#define cr_zf_24X24_S      8	//ʱ����������


#define cr_zf_24X32_T      9
#define cr_zf_24X32_A      10
#define cr_zf_16X32_F      11
#define cr_zf_24X32_S      12


#define cr_zf_40X48_T      13
#define cr_zf_40X48_A      14
#define cr_zf_24X48_F      15
#define cr_zf_40X48_S      16


#define cr_zf_40X64_T      17
#define cr_zf_40X64_A      18
#define cr_zf_32X64_F      19
#define cr_zf_48X64_S      20


#define cr_zf_32X32_TB     21	//���ƿɴ����豸ͼ��

//������
void get_cr_zf(unsigned char Sequence,unsigned long type,unsigned char*DZ_Data);

//16�����ѹ������
void hzbmp16X16( unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);//��ȡ16X16��GB18030����


/********************* ʸ�����ò��� *********************/
//ASCII����, �ֺ�: 8x16, 16x24, 16x32.
#define VEC_GBK_STY			0x1

//ʸ������
#define VEC_SONG_STY    	0x3	//����
#define VEC_BLACK_STY   	0x5	//����
#define VEC_FANGS_STY		0x6	//����
#define VEC_KAI_STY			0x7	//����

//ʸ��ASCII
#define VEC_FX_ASCII_STY	0x8	//��б
#define VEC_BX_ASCII_STY 	0x9	//��б
#define VEC_CH_ASCII_STY	0xA	//����
#define VEC_YT_ASCII_STY	0xB	//Բͷ
#define VEC_DZ_ASCII_STY	0xC	//����
#define VEC_MS_ASCII_STY	0xD	//����
#define VEC_GD_ASCII_STY	0xE	//���
#define VEC_HZ_ASCII_STY	0x10//����	
#define VEC_BZ_ASCII_STY	0x11//����
#define VEC_FT_ASCII_STY	0x12//��ͷ
#define VEC_SX_ASCII_STY	0x13//��д


/******************* ���ֵ���ģʽ���� *******************/ 

/**
 * ��ʽһ VEC_ST_MODE : ͨ��ʹ������VECFONT_ST�ṹ�����, ���ýṹ����Ϣ,
 *   ��ȡ�������ݵ�zk_buffer[]������.
 * ��ʽ�� VEC_PARM_MODE : ͨ��ָ���������е���, ��ȡ�������ݵ�pBits[]������.
 * ps: ���ַ�ʽ��ͬʱ����ʹ��, ��һʹ�����.
*/
#define VEC_ST_MODE
#define VEC_PARM_MODE

/********************* �ָ��� *********************/

#ifdef VEC_ST_MODE

    #define ZK_BUFFER_LEN   4608    //���޸Ĵ�С, Լ���� �ֺ�*�ֺ�/8.

    typedef struct vecFont
    {
        unsigned long fontCode;		//�ַ���������:GB18030, ASCII/����: unicode
        unsigned char type;
        unsigned char size;
        unsigned char thick;
        unsigned char zkBuffer[ZK_BUFFER_LEN];
    }VECFONT_ST;

    unsigned int get_font_st(VECFONT_ST * font_st);
#endif

#ifdef VEC_PARM_MODE
    
//unsigned int get_font(unsigned char *pBits,unsigned char sty,unsigned long fontCode,unsigned char width,unsigned char height, unsigned char thick);
//unsigned int get_font(unsigned char *pBits,unsigned char sty,unsigned char MSB,unsigned char LSB,unsigned char width,unsigned char height, unsigned char thick);
void get_font(unsigned char *pBits,unsigned char sty,unsigned char msb,unsigned char lsb,unsigned char len,unsigned char wth,unsigned char thick);

#endif
/********************* ʸ��������� *********************/

//ת���
unsigned int U2GB18030(unsigned int UN_CODE);//unicode תGB18030�ĺ���

unsigned int BIG52GBK( unsigned char h,unsigned char l);//BIG5תGBK�ĺ���


//�ַ�Ч��
//�Ҷȴ���
void Gray_Process(unsigned char *OutPutData ,int width,int High,unsigned char Grade);
//б�崦��
unsigned int italic(unsigned char *pInData,unsigned char *pOutData,unsigned char width,unsigned char high);
//���ߴ���
void font_hollow(unsigned char *pInData,unsigned char *pOutData,unsigned char high,unsigned char width);

#endif
