#include <stdio.h>

#define FRAME_LEN_HEAD 4

typedef unsigned char uint8_t;

static int cmd_rx_offset = 0;

void test_cmd(uint8_t *cmd_buffer_begin, int len)
{
            
            int i = 0;
            cmd_rx_offset = len;

            do
            {
                if ((*(cmd_buffer_begin + i + 0) == 0x89)
                    && (*(cmd_buffer_begin + i + 1) == 0xAB)
                    && (*(cmd_buffer_begin + i + 2) == 0xCD)
                    && (*(cmd_buffer_begin + i + 3) == 0xEF))
                {
                    printf("find the right cmd break:%d\n",i);
                    break;
                }

               // i++;
            }
            while (i++ < (cmd_rx_offset - FRAME_LEN_HEAD));

            printf("bb cmd_rx_offset:%d \n",cmd_rx_offset);
            cmd_rx_offset -= i;
            printf("cc cmd_rx_offset:%d \n",cmd_rx_offset);

}

int main()
{
        uint8_t testa[20] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0x10,0x11,0x12,0x13,0x14,0x15,0x88,0x88,0xAB,0xCD,0xEF};

        test_cmd(testa,20);
        
        return 0;
}
