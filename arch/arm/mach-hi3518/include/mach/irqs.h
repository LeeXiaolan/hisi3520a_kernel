#ifndef __HI_IRQS_H__
#define __HI_IRQS_H__

#define HI3518_IRQ_START	(0)

#define TIMER01_IRQ		(HI3518_IRQ_START + 3)
#define TIMER23_IRQ		(HI3518_IRQ_START + 4)
#define UART0_IRQ		(HI3518_IRQ_START + 5)

#define NR_IRQS			(HI3518_IRQ_START + 32)

#endif
