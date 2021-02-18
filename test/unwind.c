#include <stdio.h>
#include <unwind.h>
#include <stdlib.h>

_Unwind_Reason_Code trace(struct _Unwind_Context *ctx, void *user_data)
{
    uint64_t ip = _Unwind_GetIP(ctx);
    uint64_t cfa = _Unwind_GetCFA(ctx);

    uint32_t *num = (uint32_t*)(cfa);

    printf("ip=0x%lx cfa=0x%lx num=%d\n", ip, cfa, *num);
    return _URC_NO_REASON;
}

void callee()
{
    char s[20];
    _Unwind_Backtrace(trace, NULL);
}

int main(int argc, char const *argv[])
{
    callee();   
    return 0;
}
