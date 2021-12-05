#ifndef NETWORK_H
#define NETWORK_H




/**
 * @brief 根据域名获取ip
 * @param domain 域名
 * @param ip 返回的ip
 * @return 成功：0，失败：-1
 */
int get_ip_by_domain(const char *domain, char *ip);


/**
 * @brief 获取本机mac
 * @param 网卡号，如eth0
 * @param mac 返回的mac
 * @return 成功：0，失败：-1
 */
int get_local_mac(const char *eth_inf, char *mac);


/**
 * @brief 获取本机ip
 * @param eth_inf 网卡号，如eth0
 * @param ip 返回的ip
 * @return 成功：0，失败：-1
 */
int get_local_ip(const char *eth_inf, char *ip);


/**
 * @brief 获取本机eth0的ip
 * @param ip 返回的ip
 * @return 成功：0，失败：-1
 */
int get_eth0_ip(char *ip);


#endif // NETWORK_H
