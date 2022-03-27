#include "curl_setup.h"
#include "curl.h"
#include "urldata.h"
#include "sendf.h"
#include "multiif.h"
#include "speedcheck.h"

void Curl_speedinit(struct Curl_easy *data) {
    memset(&data->state.keeps_speed, 0, sizeof(struct curltime));
}
CURLcode Curl_speedcheck(struct Curl_easy *data, struct curltime now) {
    if((data->progress.current_speed >= 0) && data->set.low_speed_time) {
        if(data->progress.current_speed < data->set.low_speed_limit) {
            if(!data->state.keeps_speed.tv_sec) data->state.keeps_speed = now;
            else {
                timediff_t howlong = Curl_timediff(now, data->state.keeps_speed);
                if(howlong >= data->set.low_speed_time * 1000) {
                    failf(data, "Operation too slow. " "Less than %ld bytes/sec transferred the last %ld seconds", data->set.low_speed_limit, data->set.low_speed_time);
                    return CURLE_OPERATION_TIMEDOUT;
                }
            }
        } else data->state.keeps_speed.tv_sec = 0;
    }
    if(data->set.low_speed_limit) Curl_expire(data, 1000, EXPIRE_SPEEDCHECK);
    return CURLE_OK;
}