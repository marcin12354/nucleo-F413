/*
 * debug.h
 *
 *  Created on: May 24, 2024
 *      Author: marci
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

//#define debug(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)
#define debug(x) printf("\nDEBUG %s:%d: " x, __FILE__, __LINE__)

#define error(x) printf("\nDEBUG %s:%d: " x, __FILE__, __LINE__)





#endif /* INC_DEBUG_H_ */
