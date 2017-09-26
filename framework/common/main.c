/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <vfs.h>

#ifdef MESH_GATEWAY_SERVICE
#include "gateway_service.h"
#endif

extern void ota_service_init(void);
extern void version_init(void);

int aos_framework_init(void)
{
    version_init();
#ifdef MESH_GATEWAY_SERVICE
    gateway_service_init();
#endif

#ifdef AOS_FOTA
    ota_service_init();
#endif

    return 0;
}

