#include <stdio.h>
#include <stdlib.h>

typedef struct rt_cbsys_s {
	int (*callback)(void *);
	void *arg;
	struct rt_cbsys_s *next;
} rt_cbsys_t;

typedef enum {
	RT_CBSYS_START,
	RT_CBSYS_STOP,
	RT_CBSYS_POWEROFF,
	RT_CBSYS_POWERON,
	RT_CBSYS_PERIPH_SETFREQ_BEFORE,
	RT_CBSYS_PERIPH_SETFREQ_AFTER,
	RT_CBSYS_NB
} __rt_cbsys_e;

static rt_cbsys_t *cbsys_first[RT_CBSYS_NB];

void __rt_cbsys_del(__rt_cbsys_e cbsys_id, int (*cb)(void *), void *cb_arg)
{
	rt_cbsys_t *cbsys = cbsys_first[cbsys_id];
	rt_cbsys_t *prev = NULL;
	while(cbsys)
	{
		if (cbsys->callback == cb && cbsys->arg == cb_arg)
		{
			if (prev)
			{
				prev->next = cbsys->next;
			}
			else
			{
				cbsys_first[cbsys_id] = cbsys->next;
			}
			free((void *)cbsys);     
			return;
		}

		prev = cbsys;
		cbsys = cbsys->next;
	}
}

int __rt_cbsys_add(__rt_cbsys_e cbsys_id, int (*cb)(void *), void *cb_arg)
{
	rt_cbsys_t *cbsys = (rt_cbsys_t *)malloc(sizeof(rt_cbsys_t));
	if (cbsys == NULL) return -1;

	cbsys->callback = cb;
	cbsys->arg = cb_arg;
	cbsys->next = cbsys_first[cbsys_id];
	cbsys_first[cbsys_id] = cbsys;

	return 0;
}


int __rt_cbsys_exec(__rt_cbsys_e cbsys_id)
{
	rt_cbsys_t *cbsys = cbsys_first[cbsys_id];
	while (cbsys)
	{
		if (cbsys->callback(cbsys->arg)) return -1;
		cbsys = cbsys->next;
	}

	return 0;
}


void __rt_utils_init()
{
	for (int i=0; i<RT_CBSYS_NB; i++)
	{
		cbsys_first[i] = NULL;
	}
}

static int __rt_io_start(void *arg)
{
	printf("[IO] Opening UART device for IO stream\n");

	return 0;
}

static int __rt_io_stop(void *arg)
{
	printf("[IO] Close UART device for IO stream\n");

	return 0;
}
int main()
{
	__rt_utils_init();
	__rt_cbsys_add(RT_CBSYS_START, __rt_io_start, NULL);
	__rt_cbsys_add(RT_CBSYS_STOP, __rt_io_stop, NULL);

	  if (__rt_cbsys_exec(RT_CBSYS_START))
	  {

		printf("[IO] error 11111\n");
	  }
	  if (__rt_cbsys_exec(RT_CBSYS_STOP))
	  {

		printf("[IO] error 2222\n");
	  }

	__rt_cbsys_del(RT_CBSYS_START, __rt_io_start, NULL);
	__rt_cbsys_del(RT_CBSYS_STOP, __rt_io_stop, NULL);
	  return 0;
}
