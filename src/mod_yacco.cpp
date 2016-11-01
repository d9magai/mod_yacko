#include <sstream>
#include <memory>
#include <httpd.h>
#include "http_config.h"
#include <http_protocol.h>
#include <http_log.h>
#include "module_config_struct.h"
#include <aws/core/Aws.h>
#undef OK
#undef HTTP_VERSION_NOT_SUPPORTED
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/StringUtils.h>

extern "C" module AP_MODULE_DECLARE_DATA yacco_module;

#ifdef APLOG_USE_MODULE
APLOG_USE_MODULE(yacco);
#endif

const std::string HANDLER_NAME = "yacco";

/* 設定情報の生成・初期化(追加) */
static void *create_per_server_config(apr_pool_t *pool, server_rec *s)
{
    yacco_config *cfg = reinterpret_cast<yacco_config*>(apr_pcalloc(pool, sizeof(yacco_config)));

    // default value
    cfg->sha256secretkey = nullptr;
    cfg->aws_accesskey_id = nullptr;
    cfg->aws_secretaccess_key = nullptr;
    cfg->s3Client = nullptr;
    return cfg;
}

/* The sample content handler */
static int yacco_handler(request_rec *r)
{
    if (strcmp(r->handler, HANDLER_NAME.c_str())) {
        return DECLINED;
    }
    r->content_type = "text/html";

    yacco_config *conf = reinterpret_cast<yacco_config*>(ap_get_module_config(r->server->module_config, &yacco_module));

    try {
        Aws::SDKOptions options;
        Aws::InitAPI(options);

        std::string path = std::string(r->uri).substr(HANDLER_NAME.length() + 2);
        int slashpos = path.find_first_of('/');
        std::string bucket = path.substr(0, slashpos);
        std::string objectkey = path.substr(slashpos + 1);

        Aws::Client::ClientConfiguration config;
        config.scheme = Aws::Http::Scheme::HTTPS;
        config.connectTimeoutMs = 30000;
        config.requestTimeoutMs = 30000;
        config.region = Aws::Region::AP_NORTHEAST_1;

        Aws::S3::S3Client s3Client(Aws::Auth::AWSCredentials(*(conf->aws_accesskey_id), *(conf->aws_secretaccess_key)), config);
        Aws::S3::Model::GetObjectRequest getObjectRequest;
        getObjectRequest.SetBucket(bucket.c_str());
        getObjectRequest.SetKey(objectkey.c_str());

        auto getObjectOutcome = s3Client.GetObject(getObjectRequest);
        if (!getObjectOutcome.IsSuccess()) {
            std::stringstream ss;
            ss << "File download failed from s3 with error " << getObjectOutcome.GetError().GetMessage();
            throw std::runtime_error(ss.str());
        }

        std::stringstream ss;
        ss << getObjectOutcome.GetResult().GetBody().rdbuf();
        std::string data(ss.str());

        apr_bucket *b = apr_bucket_pool_create(data.c_str(), data.length(), r->pool, r->connection->bucket_alloc);
        apr_bucket_brigade *bucket_brigate = apr_brigade_create(r->pool, r->connection->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(bucket_brigate, b);
        ap_set_content_type(r, "image/jpg");
        ap_set_content_length(r, data.length());
        ap_pass_brigade(r->output_filters, bucket_brigate);

    } catch (const std::exception &e) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, APLOG_MODULE_INDEX, r, e.what());
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    return 0;//OK;
}

static void yacco_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(yacco_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

static const char *set_sha256secretkey(cmd_parms *parms, void *mconfig, const char *arg)
{
    if (strlen(arg) == 0) {
        return "sha256 secretket must be a string";
    }

    yacco_config *cfg = reinterpret_cast<yacco_config*>(ap_get_module_config(parms->server->module_config, &yacco_module));
    cfg->sha256secretkey = std::make_shared < std::string > (arg);
    return NULL;
}

static const char *set_aws_accesskey_id(cmd_parms *parms, void *mconfig, const char *arg)
{
    if (strlen(arg) == 0) {
        return "aws accesskey id must be a string";
    }

    yacco_config *cfg = reinterpret_cast<yacco_config*>(ap_get_module_config(parms->server->module_config, &yacco_module));
    Aws::StringStream ass;
    ass << arg;
    cfg->aws_accesskey_id = std::make_shared < Aws::String > (ass.str());
    return NULL;
}

static const char *set_aws_secretaccess_key(cmd_parms *parms, void *mconfig, const char *arg)
{
    if (strlen(arg) == 0) {
        return "aws secretaccess ked must be a string";
    }

    yacco_config *cfg = reinterpret_cast<yacco_config*>(ap_get_module_config(parms->server->module_config, &yacco_module));
    Aws::StringStream ass;
    ass << arg;
    cfg->aws_secretaccess_key = std::make_shared < Aws::String > (ass.str());
    return NULL;
}

/* 設定情報フック定義(追加) */
static const command_rec yacco_cmds[] =
    {
        {
        "SHA256_SECRET_KEY", set_sha256secretkey, 0, RSRC_CONF, TAKE1, "sha256 secretkey"
        },
        {
        "AWS_ACCESSKEY_ID", set_aws_accesskey_id, 0, RSRC_CONF, TAKE1, "aws accesskey id"
        },
        {
        "AWS_SECRETACCESS_KEY", set_aws_secretaccess_key, 0, RSRC_CONF, TAKE1, "aws secretaccess key"
        },
        {
        0
        },
    };

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA yacco_module =
    {
    STANDARD20_MODULE_STUFF,
    NULL,                     /* create per-dir    config structures */
    NULL,                     /* merge  per-dir    config structures */
    create_per_server_config, /* create per-server config structures */
    NULL,                     /* merge  per-server config structures */
    yacco_cmds,               /* table of config file commands       */
    yacco_register_hooks      /* register hooks                      */
    };

