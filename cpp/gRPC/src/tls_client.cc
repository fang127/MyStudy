#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "demo.grpc.pb.h"

namespace
{

using tutorial::grpcdemo::DemoService;
using tutorial::grpcdemo::HelloReply;
using tutorial::grpcdemo::HelloRequest;

std::string ReadAll(const std::string &path)
{
    std::ifstream fin(path, std::ios::binary);
    if (!fin) throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

int RunSecureHello(const std::string &target, const std::string &mode,
                   const std::string &cert_dir)
{
    grpc::SslCredentialsOptions opts;
    opts.pem_root_certs = ReadAll(cert_dir + "/ca.crt");

    const bool mtls = (mode == "mtls");
    if (mtls)
    {
        opts.pem_private_key = ReadAll(cert_dir + "/client.key");
        opts.pem_cert_chain = ReadAll(cert_dir + "/client.crt");
    }

    grpc::ChannelArguments args;
    args.SetSslTargetNameOverride("localhost");

    auto channel =
        grpc::CreateCustomChannel(target, grpc::SslCredentials(opts), args);
    auto stub = DemoService::NewStub(channel);

    grpc::ClientContext context;
    context.AddMetadata("x-token", "demo-token");
    context.AddMetadata("x-client-id",
                        mtls ? "tls-client-mtls" : "tls-client-tls");
    context.set_deadline(std::chrono::system_clock::now() +
                         std::chrono::seconds(5));

    HelloRequest req;
    req.set_name("secure-learner");
    req.set_delay_ms(0);

    HelloReply rep;
    grpc::Status status = stub->SayHello(&context, req, &rep);

    std::cout << "[tls_client] mode=" << (mtls ? "mTLS" : "TLS")
              << " status=" << status.error_code() << " "
              << status.error_message() << std::endl;

    if (!status.ok()) return 1;

    std::cout << "[tls_client] reply=" << rep.message()
              << ", server_unix_ms=" << rep.server_unix_ms() << std::endl;

    for (const auto &[k, v] : context.GetServerInitialMetadata())
    {
        std::cout << "[tls_client] initial md: "
                  << std::string(k.data(), k.length()) << "="
                  << std::string(v.data(), v.length()) << std::endl;
    }

    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    const std::string target = argc > 1 ? argv[1] : "127.0.0.1:50061";
    const std::string mode = argc > 2 ? argv[2] : "tls";
    const std::string cert_dir = argc > 3 ? argv[3] : "build/certs";

    try
    {
        return RunSecureHello(target, mode, cert_dir);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[tls_client] failed: " << ex.what() << std::endl;
        return 1;
    }
}
