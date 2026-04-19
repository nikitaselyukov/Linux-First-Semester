#!/bin/bash

cd "/home/nik/Linux.FirstSem/Paragraph 6" || exit 1

[ -e "main.c" ] && mv "main.c" "01_conditional_compilation_demo.c"
[ -e "report 6.md" ] && mv "report 6.md" "report_paragraph_6_corrected.md"

# Удалить артефакт компиляции, если он есть
[ -e "a.out" ] && rm -f "a.out"

cat > "01_conditional_compilation_demo.c" <<'EOF'
#include <stdio.h>

static void process(void)
{
#if defined(USE_GPU) && defined(USE_FLOAT)
    printf("USE_GPU and USE_FLOAT\n");
#elif defined(USE_GPU)
    printf("USE_GPU\n");
#elif defined(USE_FLOAT)
    printf("USE_FLOAT\n");
#else
    printf("None\n");
#endif
}

int main(void)
{
    process();
    return 0;
}
EOF

chmod 644 "01_conditional_compilation_demo.c"

echo "Готово."
ls -la
