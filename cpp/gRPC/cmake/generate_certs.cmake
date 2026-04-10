if(NOT OPENSSL_EXECUTABLE)
  message(FATAL_ERROR "OPENSSL_EXECUTABLE is empty. Install openssl and re-configure.")
endif()

if(NOT OUTPUT_DIR)
  message(FATAL_ERROR "OUTPUT_DIR is required")
endif()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

set(CA_KEY "${OUTPUT_DIR}/ca.key")
set(CA_CRT "${OUTPUT_DIR}/ca.crt")
set(SERVER_KEY "${OUTPUT_DIR}/server.key")
set(SERVER_CSR "${OUTPUT_DIR}/server.csr")
set(SERVER_CRT "${OUTPUT_DIR}/server.crt")
set(CLIENT_KEY "${OUTPUT_DIR}/client.key")
set(CLIENT_CSR "${OUTPUT_DIR}/client.csr")
set(CLIENT_CRT "${OUTPUT_DIR}/client.crt")
set(CA_SRL "${OUTPUT_DIR}/ca.srl")

set(SERVER_EXT "${OUTPUT_DIR}/server_ext.cnf")
set(CLIENT_EXT "${OUTPUT_DIR}/client_ext.cnf")

file(WRITE "${SERVER_EXT}" "basicConstraints=CA:FALSE\nsubjectAltName=DNS:localhost,IP:127.0.0.1\nextendedKeyUsage=serverAuth\nkeyUsage=digitalSignature,keyEncipherment\n")
file(WRITE "${CLIENT_EXT}" "basicConstraints=CA:FALSE\nextendedKeyUsage=clientAuth\nkeyUsage=digitalSignature,keyEncipherment\n")

function(run_checked)
  execute_process(
    COMMAND ${ARGN}
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
  )
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Command failed (${rc}): ${ARGN}\nstdout:\n${out}\nstderr:\n${err}")
  endif()
endfunction()

run_checked(${OPENSSL_EXECUTABLE} genrsa -out ${CA_KEY} 2048)
run_checked(${OPENSSL_EXECUTABLE} req -x509 -new -nodes -key ${CA_KEY} -sha256 -days 3650 -out ${CA_CRT} -subj /CN=grpc-demo-ca -addext basicConstraints=critical,CA:TRUE -addext keyUsage=critical,keyCertSign,cRLSign)

run_checked(${OPENSSL_EXECUTABLE} genrsa -out ${SERVER_KEY} 2048)
run_checked(${OPENSSL_EXECUTABLE} req -new -key ${SERVER_KEY} -out ${SERVER_CSR} -subj /CN=localhost)
run_checked(${OPENSSL_EXECUTABLE} x509 -req -in ${SERVER_CSR} -CA ${CA_CRT} -CAkey ${CA_KEY} -CAcreateserial -out ${SERVER_CRT} -days 3650 -sha256 -extfile ${SERVER_EXT})

run_checked(${OPENSSL_EXECUTABLE} genrsa -out ${CLIENT_KEY} 2048)
run_checked(${OPENSSL_EXECUTABLE} req -new -key ${CLIENT_KEY} -out ${CLIENT_CSR} -subj /CN=grpc-demo-client)
run_checked(${OPENSSL_EXECUTABLE} x509 -req -in ${CLIENT_CSR} -CA ${CA_CRT} -CAkey ${CA_KEY} -CAserial ${CA_SRL} -out ${CLIENT_CRT} -days 3650 -sha256 -extfile ${CLIENT_EXT})

file(REMOVE "${SERVER_CSR}" "${CLIENT_CSR}" "${SERVER_EXT}" "${CLIENT_EXT}" "${CA_SRL}")

message(STATUS "TLS/mTLS certificates generated under: ${OUTPUT_DIR}")
