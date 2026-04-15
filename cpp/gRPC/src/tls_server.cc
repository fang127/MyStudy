#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "demo.grpc.pb.h"

namespace {

using tutorial::grpcdemo::DemoService;
using tutorial::grpcdemo::HelloReply;
using tutorial::grpcdemo::HelloRequest;

std::string ReadAll(const std::string &path) {
    std::ifstream fin(path, std::ios::binary);
    if (!fin) throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

class TlsDemoService final : public DemoService::Service {
public:
    grpc::Status SayHello(grpc::ServerContext *context,
                          const HelloRequest *request,
                          HelloReply *reply) override {
        const auto it = context->client_metadata().find("x-token");
        if (it == context->client_metadata().end()) {
            return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                                "missing x-token metadata");
        }

        const std::string token(it->second.data(), it->second.length());
        if (token != "demo-token") {
            return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                                "invalid token");
        }

        context->AddInitialMetadata("x-security", "tls");
        reply->set_message("Hello, " + request->name() + " (secure channel)");
        reply->set_server_unix_ms(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
        return grpc::Status::OK;
    }
};

grpc::SslServerCredentialsOptions BuildServerSslOptions(
    const std::string &cert_dir, bool mtls_enabled) {
    grpc::SslServerCredentialsOptions opts;
    grpc::SslServerCredentialsOptions::PemKeyCertPair keycert;
    keycert.private_key = ReadAll(cert_dir + "/server.key");
    keycert.cert_chain = ReadAll(cert_dir + "/server.crt");
    opts.pem_key_cert_pairs.push_back(keycert);
    opts.pem_root_certs = ReadAll(cert_dir + "/ca.crt");
    if (mtls_enabled) {
        opts.client_certificate_request =
            GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;
    } else {
        opts.client_certificate_request =
            GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE;
    }

    return opts;
}

} // namespace

int main(int argc, char **argv) {
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50061";
    const std::string mode = argc > 2 ? argv[2] : "tls";
    const std::string cert_dir = argc > 3 ? argv[3] : "build/certs";

    const bool mtls = (mode == "mtls");

    try {
        TlsDemoService service;
        grpc::ServerBuilder builder;

        auto ssl_opts = BuildServerSslOptions(cert_dir, mtls);
        builder.AddListeningPort(server_addr,
                                 grpc::SslServerCredentials(ssl_opts));
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "[tls_server] listening on " << server_addr
                  << ", mode=" << (mtls ? "mTLS" : "TLS")
                  << ", cert_dir=" << cert_dir << std::endl;
        server->Wait();
    } catch (const std::exception &ex) {
        std::cerr << "[tls_server] startup failed: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
