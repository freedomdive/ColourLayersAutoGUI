//============================================================================
// БАРС. Библиотека функций работы с геометрическими примитивами
// Berenov D A - 1995. Нигматуллин Ф.Т. - 1996 - 2000
//============================================================================
#ifndef __RECT_H__
#define __RECT_H__

#ifdef __cplusplus
  extern "C" {
#endif

#ifdef MODULEAPI
  #undef MODULEAPI
#endif

#ifndef DFI_EXPORTS
  #define MODULEAPI __declspec(dllimport)
#else
  #define MODULEAPI __declspec(dllexport)
#endif

#include <windows.h>

//#define IsNull 0.00001
#define L2_NORM(x1, y1, x2, y2) ((int)(sqrt(pow(((x1) - (x2)), 2) + pow(((y1) - (y2)), 2)) + 0.5))

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Работа с областями
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//  Определить прямоугольник
MODULEAPI RECT Dfi_RectMake(int left, int top, int right, int bottom);

// Сдвинуть область (вычитание дельты)
#define Dfi_RectWithOffsetS(pR, pP) \
        Dfi_RectMake((pR)->left - (pP)->x, (pR)->top - (pP)->y, (pR)->right - (pP)->x, (pR)->bottom - (pP)->y)

// Сдвинуть область (прибавка дельты)
#define Dfi_RectWithOffsetA(pR, pP) \
        Dfi_RectMake((pR)->left + (pP)->x, (pR)->top + (pP)->y, (pR)->right + (pP)->x, (pR)->bottom + (pP)->y)

// Расширить(уменьшить) область 
#define Dfi_RectWithExtension(pR, pREx) \
        Dfi_RectMake((pR)->left + (pREx)->left, (pR)->top + (pREx)->top, (pR)->right + (pREx)->right, (pR)->bottom + (pREx)->bottom)

// Расширить(уменьшить) область 
#define Dfi_RectResize(pR, Value) \
        Dfi_RectMake((pR)->left - Value, (pR)->top - Value, (pR)->right + Value, (pR)->bottom + Value)

// Выравнять область по четным границам
MODULEAPI void Dfi_RectWithEven(PRECT pR);

#define Dfi_RectWidth(pR)  ((pR)->right - (pR)->left)
#define Dfi_RectHeight(pR) ((pR)->bottom - (pR)->top)
#define Dfi_RectSquare(pR) (Dfi_RectWidth(pR)*Dfi_RectHeight(pR))
#define Dfi_RectCenter(pR) Dfi_PointMake(((pR)->left + (pR)->right) >> 1, ((pR)->top + (pR)->bottom) >> 1)

// Сравнить равенство областей
#define Dfi_RectsIsEqual(pR1, pR2) (((pR1)->left  == (pR2)->left)  && ((pR1)->top    == (pR2)->top) && \
                                    ((pR1)->right == (pR2)->right) && ((pR1)->bottom == (pR2)->bottom))

// Определить ошибочны ли данные RECT'a (на <= 0)
// Результат: 1 - ошибочны, 0 - нет
MODULEAPI int  Dfi_RectIsInvalid(PRECT pR);

// Определить не пересекаются ли области
// Результат:
//   0    - не пересекаются
//   1,-1 - вхождение или полное совпадение
//   2,-2 - пересечение сторон или полное совпадение
//   3,-3 - 1 вершина одной области находится в другой области
MODULEAPI int Dfi_RectsIsIntersection(PRECT b1, PRECT b2);

// Исправить RECT в случае, если границы перепутаны
MODULEAPI int Dfi_RectReverse(PRECT pR);

// Исправить границы области, вернуть к положительным значениям (если < 0)
MODULEAPI int Dfi_RectCorrect(PRECT pR);

// Проверка нахождения прямоугольника <a> в нужных пределах
// и его возврат в эти пределы
MODULEAPI void Dfi_RectComeBackToArea(RECT *a, int left, int top, int right, int bottom);
#define Dfi_RectComeBackToRect(a, b) \
        Dfi_RectComeBackToArea(a, (b)->left, (b)->top, (b)->right, (b)->bottom)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Работа с точками
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Создать точку
MODULEAPI POINT Dfi_PointMake(int x,int y);

// Выравнять точку по четным границам
MODULEAPI void Dfi_PointWithEven(PPOINT p);

// Сравнить равенство областей
#define Dfi_PointsIsEqual(pP1, pP2) (((pP1)->x == (pP2)->x) && ((pP1)->y == (pP2)->y))

// Нахождение точки в прямоугольнике: BOOL Dfi_PointIsInRect(PPOINT p, PRECT r)
#define Dfi_PointIsInRect(p, r) \
        ((p)->x >= (r)->left && (p)->x <= (r)->right && (p)->y >= (r)->top  && (p)->y <= (r)->bottom)

// Нахождение точки в прямоугольнике: BOOL PointInRect(int p_x, int p_y, PRECT r)
#define Dfi_PointIsInRectEx(p_x, p_y, r) \
        ((p_x) >= (r)->left && (p_x) <= (r)->right && (p_y) >= (r)->top  && (p_y) <= (r)->bottom)

// Проверка нахождения точки в прямоугольнике и ее возврат в эти пределы
MODULEAPI void Dfi_PointComeBackToArea(PPOINT pP, PRECT pR);

// Найти угол между тремя точками (Laob)
MODULEAPI float Dfi_PointsCalcAngle(POINT a, POINT o, POINT b);

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Работа с линиями
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// Функция возвращающая точку пересечения двух прямых
// по заданным коэффициентам уравнения
MODULEAPI POINT Dfi_LinesCrossByEquation(float A1, float B1, int C1, 
                                         float A2, float B2, int C2, int mode);

// Функция возвращающая коэффициенты уравнения по 2-м точкам
MODULEAPI void Dfi_LinesEquationByCross(float *pA, float *pB, int *pC, POINT P1, POINT P2);

// Функция возвращающая точку пересечения двух прямых
// по заданным парам точек (p1, p2) и (p3, p4)
MODULEAPI POINT Dfi_LinesGetPointOfCross(POINT p1, POINT p2, POINT p3, POINT p4);

// Расстояние от точки до прямой
MODULEAPI int Dfi_DistPointToLine(float x, float y, float a, float b, int c);

#ifdef __cplusplus
  }	// extern "C"
#endif

#endif