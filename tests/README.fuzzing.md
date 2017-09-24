# Compilation
```bash
CC="<AFL_ROOT>/afl-gcc" CXX="<AFL_ROOT>/afl-g++" ./configure --disable-shared
make
```

# Generating Seeds
## Generate
A simple way to get started is to apply this or similar patches and run the unit tests.
```patch
diff --git a/src/modbus-tcp.c b/src/modbus-tcp.c
index a0846f2..1f1b5df 100644
--- a/src/modbus-tcp.c
+++ b/src/modbus-tcp.c
@@ -163,12 +163,20 @@ static int _modbus_tcp_send_msg_pre(uint8_t *req, int req_length)
     return req_length;
 }
 
+int fc = 0;
+
 static ssize_t _modbus_tcp_send(modbus_t *ctx, const uint8_t *req, int req_length)
 {
     /* MSG_NOSIGNAL
        Requests not to send SIGPIPE on errors on stream oriented
        sockets when the other end breaks the connection.  The EPIPE
        error is still returned. */
+      char *filename = (char*)malloc(1024 * sizeof(char));
+      sprintf(filename, "fuzzing-seeds/test-case-%d.tcp.raw", fc++);
+      FILE* f = fopen(filename, "w");
+      fwrite(req, sizeof(uint8_t), req_length, f);
+      fclose(f);
+      free(filename);
     return send(ctx->s, (const char *)req, req_length, MSG_NOSIGNAL);
 }
```

## Minimise
AFL has a tool to minimise the number of seeds by removing duplicate or "uninteresting" seeds. This improves the later fuzzing startup times.
```bash
<AFL_ROOT>/afl-cmin -i fuzzing-seeds -o fuzzing-seeds-cmin tests/fuzzing_harness
```

# Fuzzing
```bash
<AFL_ROOT>/afl-fuzz -i fuzzing-seeds-cmin -o fuzzing-results -M fuzzerMaster -- tests/fuzzing_harness
<AFL_ROOT>/afl-fuzz -i fuzzing-seeds-cmin -o fuzzing-results -S fuzzerSlaveX -- tests/fuzzing_harness
```

# Notes
* AFL does not like to be on the PATH during configure/make