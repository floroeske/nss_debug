#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <nss.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>


enum nss_status
_nss_debug_sethostent(int stayopen)
{
    fprintf(stderr, "NSS DEBUG: Called %s with args (stayopen: %d)\n", __FUNCTION__, stayopen);
    return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_debug_endhostent(void)
{
    fprintf(stderr, "NSS DEBUG: Called %s\n", __FUNCTION__);
    return NSS_STATUS_NOTFOUND;
}


enum nss_status
_nss_debug_gethostent_r(struct hostent *host, char *buffer, size_t buflen, int *errnop, int *h_errnop)
{
    fprintf(stderr, "NSS DEBUG: Called %s\n", __FUNCTION__);
    return NSS_STATUS_NOTFOUND;
}


enum nss_status
_nss_debug_gethostbyaddr_r(const void *addr, socklen_t addrlen, int af, struct hostent *host, char *buffer,
                           size_t buflen, int *errnop, int *h_errnop)
{
    fprintf(stderr, "NSS DEBUG: Called %s with args (addr: %s)\n", __FUNCTION__,
            inet_ntoa(*(struct in_addr *)addr));

    fprintf(stderr, "NSS DEBUG: Returning\n");
    return NSS_STATUS_NOTFOUND;

    // Ensure that the address is an IPv4 address
    if (af != AF_INET) {
        return NSS_STATUS_NOTFOUND;
    }

    // Check if the address matches the target IP address
    struct in_addr target_addr;
    if (inet_pton(AF_INET, "192.168.70.3", &target_addr) != 1) {
        return NSS_STATUS_NOTFOUND;
    }
    if (memcmp(addr, &target_addr, sizeof(target_addr)) != 0) {
        return NSS_STATUS_NOTFOUND;
    }

    // Fill in the hostent structure
    host->h_name = "zeus-17535";
    host->h_aliases = NULL;
    host->h_addrtype = AF_INET;
    host->h_length = sizeof(struct in_addr);
    host->h_addr_list = malloc(2 * sizeof(char *));
    if (host->h_addr_list == NULL) {
        return NSS_STATUS_TRYAGAIN;
    }
    host->h_addr_list[0] = malloc(sizeof(struct in_addr));
    if (host->h_addr_list[0] == NULL) {
        free(host->h_addr_list);
        return NSS_STATUS_TRYAGAIN;
    }
    memcpy(host->h_addr_list[0], &target_addr, sizeof(target_addr));
    host->h_addr_list[1] = NULL;

    return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_debug_gethostbyname2_r(const char *name, int af, struct hostent *host, char *buffer, size_t buflen,
                            int *errnop, int *h_errnop)
{
    fprintf(stderr, "NSS DEBUG: Called %s with args (name: %s)\n", __FUNCTION__, name);
    return NSS_STATUS_NOTFOUND;
}

int resolve_dot_local_host(const char *hostname, char *ipaddr, size_t ipaddr_len) {
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[INET6_ADDRSTRLEN];

    // Append ".local" to the hostname
    char hostname_local[strlen(hostname) + strlen(".local") + 1];
    sprintf(hostname_local, "%s.local", hostname);

    // Set up hints for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Use wildcard IP address

    // Call getaddrinfo to resolve the hostname to IP address
    if ((errcode = getaddrinfo(hostname_local, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errcode));
        return -1;
    }

    // Loop over the results and convert each to a string
    while (res != NULL) {
        inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addrstr, sizeof(addrstr));
        strncpy(ipaddr, addrstr, ipaddr_len);
        res = res->ai_next;
    }

    // Free the results list
    freeaddrinfo(res);

    return 0;
}

enum nss_status
_nss_debug_gethostbyname4_r(const char *name, struct gaih_addrtuple **pat, char *buffer, size_t buflen,
                            int *errnop, int *herrnop, int32_t *ttlp)
{
    fprintf(stderr, "NSS DEBUG: Called %s with args (name: %s)\n", __FUNCTION__, name);

    if (strstr(name, ".local") != NULL) {
        fprintf(stderr, "NSS DEBUG: Ignoring name containing '.local'\n");
        return NSS_STATUS_NOTFOUND;
    }

    if (strchr(name, '.') != NULL) {
        fprintf(stderr, "NSS DEBUG: Ignoring name containing '.'\n");
        return NSS_STATUS_NOTFOUND;
    }

    char ipaddr[INET6_ADDRSTRLEN];
    if (resolve_dot_local_host(name, ipaddr, sizeof(ipaddr)) == 0) {
        fprintf(stderr, "NSS DEBUG: IP address of %s.local is: %s\n", name, ipaddr);
    } else {
        fprintf(stderr, "NSS DEBUG: Failed to resolve hostname\n");
    }

    // Allocate memory for the result
    struct gaih_addrtuple *result = malloc(sizeof(struct gaih_addrtuple));
    if (result == NULL) {
        return NSS_STATUS_TRYAGAIN;  // Memory allocation error
    }

    // Initialize the result with the IP address 192.168.70.1
    struct in_addr addr;
    if (inet_pton(AF_INET, ipaddr, &addr) != 1) {
        free(result);
        return NSS_STATUS_UNAVAIL;  // Invalid IP address format
    }
    result->next = NULL;
    result->name = NULL;
    result->family = AF_INET;
    result->scopeid = 0;
    memcpy(&result->addr, &addr, sizeof(struct in_addr));

    // Set the output parameters
    *pat = result;
    *errnop = 0;
    *herrnop = 0;
    if (ttlp != NULL) {
        *ttlp = -1;
    }

    fprintf(stderr, "NSS DEBUG: NSS_STATUS_SUCCESS\n");
    return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_debug_gethostbyname_r(const char *name, struct hostent *host, char *buffer, size_t buflen,
                           int *errnop, int *h_errnop)
{
    fprintf(stderr, "NSS DEBUG: Called %s with args (name: %s)\n", __FUNCTION__, name);
    return NSS_STATUS_NOTFOUND;
}
