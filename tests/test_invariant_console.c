#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

static jmp_buf env;

static void signal_handler(int sig) {
    longjmp(env, 1);
}

START_TEST(test_console_printf_no_buffer_overflow)
{
    // Invariant: Buffer reads never exceed the declared length
    const char *payloads[] = {
        "%1023s",  // Boundary: fills exactly 1023 chars + null terminator
        "%1024s",  // Exploit: exceeds buffer by 1 byte (1024 chars + null = 1025 bytes)
        "%2000s",  // Exploit: significantly exceeds buffer
        "normal",  // Valid input
        NULL
    };
    
    // Remove NULL from count
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]) - 1;
    
    // Setup signal handler for SIGSEGV/SIGABRT
    struct sigaction sa;
    struct sigaction old_sa;
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGSEGV, &sa, &old_sa);
    sigaction(SIGABRT, &sa, NULL);
    
    for (int i = 0; i < num_payloads; i++) {
        // Prepare large argument for format strings
        char large_arg[2001];
        memset(large_arg, 'A', sizeof(large_arg) - 1);
        large_arg[sizeof(large_arg) - 1] = '\0';
        
        if (setjmp(env) == 0) {
            // Call the actual production function
            console_printf(false, payloads[i], large_arg);
            // If we get here without signal, test passes for this payload
        } else {
            // Signal caught - buffer overflow occurred
            ck_abort_msg("Buffer overflow detected with payload: %s", payloads[i]);
        }
    }
    
    // Restore original signal handler
    sigaction(SIGSEGV, &old_sa, NULL);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_console_printf_no_buffer_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}