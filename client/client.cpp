#include <iostream>
#include <cstdlib>
#include <cstring>

#include <zmq.hpp>

enum command_line_args
{
    ARG_SELF,
    ARG_SERVER,
    ARG_MESSAGE,
    ARG_COUNT
};

int main(int argc, char *argv[])
{
    if (argc != ARG_COUNT) {
        std::cerr << argv[ARG_SELF] << ": Invalid command line. Usage:\n"
                  << argv[ARG_SELF] << " zmq_endpoint message" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        zmq::context_t context(1);

        zmq::socket_t socket(context, ZMQ_REQ);

        std::cerr << "Connecting to " << argv[ARG_SERVER] << std::endl;
        socket.connect(argv[ARG_SERVER]);

        std::cerr << "Sending " << argv[ARG_MESSAGE] << std::endl;
        if (!socket.send(argv[ARG_MESSAGE], std::strlen(argv[ARG_MESSAGE]))) {
            throw zmq::error_t();
        }

        std::cerr << "Waiting for response" << std::endl;
        zmq::message_t response;
        if (!socket.recv(&response)) {
            throw zmq::error_t();
        }

        std::cerr.write(response.data<const char>(), response.size());
        std::cerr << std::endl;

        if (std::strncmp("OK", response.data<const char>(), 2) == 0) {
            return EXIT_SUCCESS;
        }

    } catch (const std::exception &ex) {
        std::cerr << argv[ARG_SELF] << ex.what() << std::endl;
    }

    return EXIT_FAILURE;
}
