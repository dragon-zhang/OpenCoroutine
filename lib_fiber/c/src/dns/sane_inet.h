#ifndef	__SANE_INET_INCLUDE_H__
#define	__SANE_INET_INCLUDE_H__

#include "../../include/fiber/fiber_define.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * ��IP��ַת�����ַ�����ʽ
 * @param src {const unsigned char*} struct in_addr in.s_addr �������ڴ��ʾ
 * @param dst {char *} �洢ת�����
 * @param size {size_t} dst �Ŀռ��С
 * @return {const char*} NULL: error; !NULL: ok
 */
const char *inet_ntop4(const unsigned char *src, char *dst, size_t size);

/**
 * ��IP��ַת�����ַ�����ʽ
 * @param in {struct in_addr}
 * @param dst {char *} �洢ת�����
 * @param size {size_t} dst �Ŀռ��С
 * @return {const char*} NULL: error; !NULL: ok
 */
const char *sane_inet_ntoa(struct in_addr in, char *dst, size_t size);

/**
 * �жϸ������ַ����Ƿ�����ȷ�� ip ��ַ
 * @param ip {const char *ip}
 * @return {int} != 0: ��; == 0: ��
 */
int is_ip(const char *ip);
int is_ipv4(const char *ip);
int is_ipv6(const char *ip);

/**
 * �ж������� ip ��ַ�Ƿ���� xxx.xxx.xxx.xxx:port ��ʽ
 * @param addr {const char*} IP:PORT ��ַ
 * @return {int} 1: ����, 0: ������
 */
int ipv4_addr_valid(const char *addr);

#ifdef	__cplusplus
}
#endif

#endif

