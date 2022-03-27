#include <stdbool.h>
#include "curl_setup.h"
#include "urldata.h"
#include "sendf.h"
#include "multiif.h"
#include "progress.h"
#include "timeval.h"
#include "curl_printf.h"

#define MIN_RATE_LIMIT_PERIOD 3000
#ifndef CURL_DISABLE_PROGRESS_METER

static void time2str(char *r, curl_off_t seconds) {
    curl_off_t h;
    if(seconds <= 0) {
        strcpy(r, "--:--:--");
        return;
    }
    h = seconds / CURL_OFF_T_C(3600);
    if(h <= CURL_OFF_T_C(99)) {
        curl_off_t m = (seconds - (h*CURL_OFF_T_C(3600))) / CURL_OFF_T_C(60);
        curl_off_t s = (seconds - (h*CURL_OFF_T_C(3600))) - (m*CURL_OFF_T_C(60));
        msnprintf(r, 9, "%2" CURL_FORMAT_CURL_OFF_T ":%02" CURL_FORMAT_CURL_OFF_T ":%02" CURL_FORMAT_CURL_OFF_T, h, m, s);
    } else {
        curl_off_t d = seconds / CURL_OFF_T_C(86400);
        h = (seconds - (d*CURL_OFF_T_C(86400))) / CURL_OFF_T_C(3600);
        if(d <= CURL_OFF_T_C(999)) msnprintf(r, 9, "%3" CURL_FORMAT_CURL_OFF_T "d %02" CURL_FORMAT_CURL_OFF_T "h", d, h);
        else msnprintf(r, 9, "%7" CURL_FORMAT_CURL_OFF_T "d", d);
    }
}
static char *max5data(curl_off_t bytes, char *max5) {
    #define ONE_KILOBYTE  CURL_OFF_T_C(1024)
    #define ONE_MEGABYTE (CURL_OFF_T_C(1024) * ONE_KILOBYTE)
    #define ONE_GIGABYTE (CURL_OFF_T_C(1024) * ONE_MEGABYTE)
    #define ONE_TERABYTE (CURL_OFF_T_C(1024) * ONE_GIGABYTE)
    #define ONE_PETABYTE (CURL_OFF_T_C(1024) * ONE_TERABYTE)
    if(bytes < CURL_OFF_T_C(100000)) msnprintf(max5, 6, "%5" CURL_FORMAT_CURL_OFF_T, bytes);
    else if(bytes < CURL_OFF_T_C(10000) * ONE_KILOBYTE) msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "k", bytes/ONE_KILOBYTE);
    else if(bytes < CURL_OFF_T_C(100) * ONE_MEGABYTE)
        msnprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE,
                 (bytes%ONE_MEGABYTE) / (ONE_MEGABYTE/CURL_OFF_T_C(10)));
#if (CURL_SIZEOF_CURL_OFF_T > 4)
    else if(bytes < CURL_OFF_T_C(10000) * ONE_MEGABYTE) msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);
    else if(bytes < CURL_OFF_T_C(100) * ONE_GIGABYTE)
        msnprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0" CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE,
                  (bytes%ONE_GIGABYTE) / (ONE_GIGABYTE/CURL_OFF_T_C(10)) );
    else if(bytes < CURL_OFF_T_C(10000) * ONE_GIGABYTE) msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE);
    else if(bytes < CURL_OFF_T_C(10000) * ONE_TERABYTE) msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "T", bytes/ONE_TERABYTE);
    else msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "P", bytes/ONE_PETABYTE);
#else
    else msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);
#endif
    return max5;
}
#endif
int Curl_pgrsDone(struct connectdata *conn) {
    int rc;
    struct Curl_easy *data = conn->data;
    data->progress.lastshow = 0;
    rc = Curl_pgrsUpdate(conn);
    if(rc) return rc;
    if(!(data->progress.flags & PGRS_HIDE) && !data->progress.callback) fprintf(data->set.err, "\n");
    data->progress.speeder_c = 0;
    return 0;
}
void Curl_pgrsResetTransferSizes(struct Curl_easy *data) {
    Curl_pgrsSetDownloadSize(data, -1);
    Curl_pgrsSetUploadSize(data, -1);
}
void Curl_pgrsTime(struct Curl_easy *data, timerid timer) {
    struct curltime now = Curl_now();
    timediff_t *delta = NULL;
    switch(timer) {
        case TIMER_STARTOP: data->progress.t_startop = now; break;
        case TIMER_STARTSINGLE:
            data->progress.t_startsingle = now;
            data->progress.is_t_startransfer_set = false;
            break;
        case TIMER_STARTACCEPT: data->progress.t_acceptdata = now; break;
        case TIMER_NAMELOOKUP: delta = &data->progress.t_nslookup; break;
        case TIMER_CONNECT: delta = &data->progress.t_connect; break;
        case TIMER_APPCONNECT: delta = &data->progress.t_appconnect; break;
        case TIMER_PRETRANSFER: delta = &data->progress.t_pretransfer; break;
        case TIMER_STARTTRANSFER:
            delta = &data->progress.t_starttransfer;
            if(data->progress.is_t_startransfer_set) return;
            else {
              data->progress.is_t_startransfer_set = true;
              break;
            }
        case TIMER_REDIRECT: data->progress.t_redirect = Curl_timediff_us(now, data->progress.start); break;
    }
    if(delta) {
        timediff_t us = Curl_timediff_us(now, data->progress.t_startsingle);
        if(us < 1) us = 1;
        *delta += us;
    }
}
void Curl_pgrsStartNow(struct Curl_easy *data) {
    data->progress.speeder_c = 0;
    data->progress.start = Curl_now();
    data->progress.is_t_startransfer_set = false;
    data->progress.ul_limit_start.tv_sec = 0;
    data->progress.ul_limit_start.tv_usec = 0;
    data->progress.dl_limit_start.tv_sec = 0;
    data->progress.dl_limit_start.tv_usec = 0;
    data->progress.downloaded = 0;
    data->progress.uploaded = 0;
    data->progress.flags &= PGRS_HIDE|PGRS_HEADERS_OUT;
    Curl_ratelimit(data, data->progress.start);
}
timediff_t Curl_pgrsLimitWaitTime(curl_off_t cursize, curl_off_t startsize, curl_off_t limit, struct curltime start, struct curltime now) {
    curl_off_t size = cursize - startsize;
    timediff_t minimum;
    timediff_t actual;
    if(!limit || !size) return 0;
    if(size < CURL_OFF_T_MAX/1000) minimum = (timediff_t) (CURL_OFF_T_C(1000) * size / limit);
    else {
        minimum = (timediff_t) (size / limit);
        if(minimum < TIMEDIFF_T_MAX/1000) minimum *= 1000;
        else minimum = TIMEDIFF_T_MAX;
    }
    actual = Curl_timediff(now, start);
    if(actual < minimum) return (minimum - actual);
    return 0;
}
void Curl_pgrsSetDownloadCounter(struct Curl_easy *data, curl_off_t size) {
  data->progress.downloaded = size;
}
void Curl_ratelimit(struct Curl_easy *data, struct curltime now) {
    if(data->set.max_recv_speed > 0) {
        if(Curl_timediff(now, data->progress.dl_limit_start) >= MIN_RATE_LIMIT_PERIOD) {
            data->progress.dl_limit_start = now;
            data->progress.dl_limit_size = data->progress.downloaded;
        }
    }
    if(data->set.max_send_speed > 0) {
        if(Curl_timediff(now, data->progress.ul_limit_start) >= MIN_RATE_LIMIT_PERIOD) {
            data->progress.ul_limit_start = now;
            data->progress.ul_limit_size = data->progress.uploaded;
        }
    }
}
void Curl_pgrsSetUploadCounter(struct Curl_easy *data, curl_off_t size) {
    data->progress.uploaded = size;
}
void Curl_pgrsSetDownloadSize(struct Curl_easy *data, curl_off_t size) {
    if(size >= 0) {
        data->progress.size_dl = size;
        data->progress.flags |= PGRS_DL_SIZE_KNOWN;
    } else {
        data->progress.size_dl = 0;
        data->progress.flags &= ~PGRS_DL_SIZE_KNOWN;
    }
}
void Curl_pgrsSetUploadSize(struct Curl_easy *data, curl_off_t size) {
    if(size >= 0) {
        data->progress.size_ul = size;
        data->progress.flags |= PGRS_UL_SIZE_KNOWN;
    } else {
        data->progress.size_ul = 0;
        data->progress.flags &= ~PGRS_UL_SIZE_KNOWN;
    }
}
static bool progress_calc(struct connectdata *conn, struct curltime now) {
    curl_off_t timespent;
    curl_off_t timespent_ms;
    struct Curl_easy *data = conn->data;
    curl_off_t dl = data->progress.downloaded;
    curl_off_t ul = data->progress.uploaded;
    bool timetoshow = FALSE;
    data->progress.timespent = Curl_timediff_us(now, data->progress.start);
    timespent = (curl_off_t)data->progress.timespent/1000000;
    timespent_ms = (curl_off_t)data->progress.timespent/1000;
    if(dl < CURL_OFF_T_MAX/1000) data->progress.dlspeed = (dl * 1000 / (timespent_ms>0?timespent_ms:1));
    else data->progress.dlspeed = (dl / (timespent>0?timespent:1));
    if(ul < CURL_OFF_T_MAX/1000) data->progress.ulspeed = (ul * 1000 / (timespent_ms>0?timespent_ms:1));
    else data->progress.ulspeed = (ul / (timespent>0?timespent:1));
    if(data->progress.lastshow != now.tv_sec) {
        int countindex;
        int nowindex = data->progress.speeder_c% CURR_TIME;
        data->progress.lastshow = now.tv_sec;
        timetoshow = TRUE;
        data->progress.speeder[ nowindex ] = data->progress.downloaded + data->progress.uploaded;
        data->progress.speeder_time [ nowindex ] = now;
        data->progress.speeder_c++;
        countindex = ((data->progress.speeder_c >= CURR_TIME) ? CURR_TIME : data->progress.speeder_c) - 1;
        if(countindex) {
            int checkindex;
            timediff_t span_ms;
            checkindex = (data->progress.speeder_c >= CURR_TIME) ? data->progress.speeder_c%CURR_TIME : 0;
            span_ms = Curl_timediff(now, data->progress.speeder_time[checkindex]);
            if(0 == span_ms) span_ms = 1;
            {
            curl_off_t amount = data->progress.speeder[nowindex] - data->progress.speeder[checkindex];
            if(amount > CURL_OFF_T_C(4294967)) data->progress.current_speed = (curl_off_t)((double)amount/((double)span_ms/1000.0));
            else data->progress.current_speed = amount*CURL_OFF_T_C(1000)/span_ms;
            }
        } else data->progress.current_speed = data->progress.ulspeed + data->progress.dlspeed;
    }
    return timetoshow;
}
#ifndef CURL_DISABLE_PROGRESS_METER
static void progress_meter(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    char max5[6][10];
    curl_off_t dlpercen = 0;
    curl_off_t ulpercen = 0;
    curl_off_t total_percen = 0;
    curl_off_t total_transfer;
    curl_off_t total_expected_transfer;
    char time_left[10];
    char time_total[10];
    char time_spent[10];
    curl_off_t ulestimate = 0;
    curl_off_t dlestimate = 0;
    curl_off_t total_estimate;
    curl_off_t timespent = (curl_off_t)data->progress.timespent / 1000000;
    if(!(data->progress.flags & PGRS_HEADERS_OUT)) {
        if(data->state.resume_from) fprintf(data->set.err, "** Resuming transfer from byte position %" CURL_FORMAT_CURL_OFF_T "\n", data->state.resume_from);
        fprintf(data->set.err, "  %% Total    %% Received %% Xferd  Average Speed   " "Time    Time     Time  Current\n"
                "                                 Dload  Upload   " "Total   Spent    Left  Speed\n");
        data->progress.flags |= PGRS_HEADERS_OUT;
    }
    if((data->progress.flags & PGRS_UL_SIZE_KNOWN) && (data->progress.ulspeed > CURL_OFF_T_C(0))) {
        ulestimate = data->progress.size_ul / data->progress.ulspeed;
        if(data->progress.size_ul > CURL_OFF_T_C(10000)) ulpercen = data->progress.uploaded / (data->progress.size_ul/CURL_OFF_T_C(100));
        else if(data->progress.size_ul > CURL_OFF_T_C(0)) ulpercen = (data->progress.uploaded*100) / data->progress.size_ul;
    }
    if((data->progress.flags & PGRS_DL_SIZE_KNOWN) && (data->progress.dlspeed > CURL_OFF_T_C(0))) {
        dlestimate = data->progress.size_dl / data->progress.dlspeed;
        if(data->progress.size_dl > CURL_OFF_T_C(10000)) dlpercen = data->progress.downloaded / (data->progress.size_dl/CURL_OFF_T_C(100));
        else if(data->progress.size_dl > CURL_OFF_T_C(0)) dlpercen = (data->progress.downloaded*100) / data->progress.size_dl;
    }
    total_estimate = ulestimate>dlestimate?ulestimate:dlestimate;
    time2str(time_left, total_estimate > 0 ? (total_estimate - timespent) : 0);
    time2str(time_total, total_estimate);
    time2str(time_spent, timespent);
    total_expected_transfer = ((data->progress.flags & PGRS_UL_SIZE_KNOWN) ? data->progress.size_ul : data->progress.uploaded) +
                               ((data->progress.flags & PGRS_DL_SIZE_KNOWN) ? data->progress.size_dl : data->progress.downloaded);
    total_transfer = data->progress.downloaded + data->progress.uploaded;
    if(total_expected_transfer > CURL_OFF_T_C(10000)) total_percen = total_transfer / (total_expected_transfer/CURL_OFF_T_C(100));
    else if(total_expected_transfer > CURL_OFF_T_C(0)) total_percen = (total_transfer*100) / total_expected_transfer;
    fprintf(data->set.err, "\r" "%3" CURL_FORMAT_CURL_OFF_T " %s  " "%3" CURL_FORMAT_CURL_OFF_T " %s  "
            "%3" CURL_FORMAT_CURL_OFF_T " %s  %s  %s %s %s %s %s", total_percen, max5data(total_expected_transfer, max5[2]), dlpercen,
            max5data(data->progress.downloaded, max5[0]), ulpercen, max5data(data->progress.uploaded, max5[1]), max5data(data->progress.dlspeed, max5[3]),
            max5data(data->progress.ulspeed, max5[4]), time_total, time_spent, time_left, max5data(data->progress.current_speed, max5[5]));
    fflush(data->set.err);
}
#else
#define progress_meter(x) Curl_nop_stmt
#endif
int Curl_pgrsUpdate(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    struct curltime now = Curl_now();
    bool showprogress = progress_calc(conn, now);
    if(!(data->progress.flags & PGRS_HIDE)) {
        if(data->set.fxferinfo) {
            int result;
            Curl_set_in_callback(data, true);
            result = data->set.fxferinfo(data->set.progress_client, data->progress.size_dl, data->progress.downloaded, data->progress.size_ul, data->progress.uploaded);
            Curl_set_in_callback(data, false);
            if(result != CURL_PROGRESSFUNC_CONTINUE) {
                if(result) failf(data, "Callback aborted");
                return result;
            }
        } else if(data->set.fprogress) {
            int result;
            Curl_set_in_callback(data, true);
            result = data->set.fprogress(data->set.progress_client, (double)data->progress.size_dl, (double)data->progress.downloaded, (double)data->progress.size_ul,
                                         (double)data->progress.uploaded);
            Curl_set_in_callback(data, false);
            if(result != CURL_PROGRESSFUNC_CONTINUE) {
                if(result) failf(data, "Callback aborted");
                return result;
            }
        }
        if(showprogress) progress_meter(conn);
    }
    return 0;
}