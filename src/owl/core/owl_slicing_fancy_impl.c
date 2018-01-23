/*
 * OWL - an OCaml numerical library for scientific computing
 * Copyright (c) 2016-2018 Liang Wang <liang.wang@cl.cam.ac.uk>
 */

#ifdef OWL_ENABLE_TEMPLATE


// slice x based on the fancy slice definition and save to y.
void FUNCTION (c, fancy) (struct fancy_pair *p) {
  TYPE *x = (TYPE *) p->x;
  TYPE *y = (TYPE *) p->y;
  const int d = p->dep;
  const int n = p->n[d];
  const int e = d + d + d;
  const int a = p->slice[e];
  const int b = p->slice[e + 1];
  const int c = p->slice[e + 2];
  const int incx = p->incx[d];
  const int incy = p->incy[d];
  const int save_posx = p->posx;
  const int save_posy = p->posy;
  p->posx += p->ofsx[d];
  p->posy += p->ofsy[d];

  if (p->dep == p->dim - 1) {
    int posx = p->posx;
    int posy = p->posy;

    if (a < 0) {
      // fancy slicing, (a, b, c) = (_, start, stop) in index
      for (int i = b; i <= c; i++) {
        //printf("@@@ b:%i c:%i ofsx:%i incx:%i ofsy:%i incy:%i posx:%lld posy:%i\n", b, c, p->ofsx[d], incx, p->ofsy[d], incy, (save_posx + incx * p->index[i]), posy);
        posx = save_posx + incx * p->index[i];
        MAPFUN (*(x + posx), *(y + posy));
        posy += incy;
      }
    }
    else {
      // basic slicing, (a, b, c) = (start, stop, step)
      for (int i = 0; i < n; i++) {
        //printf("=== a:%i b:%i c:%i ofsx:%i incx:%i ofsy:%i incy:%i posx:%lld posy:%i\n", a, b, c, p->ofsx[d], incx, p->ofsy[d], incy, posx, posy);
        MAPFUN (*(x + posx), *(y + posy));
        posx += incx;
        posy += incy;
      }
    }
  }
  else {
    if (a < 0) {
      // fancy slicing, (a, b, c) = (_, start, stop) in index
      for (int i = b; i <= c; i++) {
        //printf("+++ b:%i c:%i posx:%lld\n", b, c, p->index[i]);
        p->posx = save_posx + incx * p->index[i];
        p->dep += 1;
        FUNCTION (c, fancy) (p);
        p->dep -= 1;
        p->posy += incy;
      }
    }
    else {
      // basic slicing, (a, b, c) = (start, stop, step)
      for (int i = 0; i < n; i++) {
        p->dep += 1;
        FUNCTION (c, fancy) (p);
        p->dep -= 1;
        p->posx += incx;
        p->posy += incy;
      }
    }
  }

  p->posx = save_posx;
  p->posy = save_posy;
}


// stub function
value FUNCTION (stub, fancy) (value vX, value vY, value vA, value vB) {
  struct caml_ba_array *X = Caml_ba_array_val(vX);
  TYPE *X_data = (TYPE *) X->data;

  struct caml_ba_array *Y = Caml_ba_array_val(vY);
  TYPE *Y_data = (TYPE *) Y->data;

  struct caml_ba_array *A = Caml_ba_array_val(vA);
  int64_t *slice = (int64_t *) A->data;

  struct caml_ba_array *B = Caml_ba_array_val(vB);
  int64_t *index = (int64_t *) B->data;

  struct fancy_pair * fp = calloc(1, sizeof(struct fancy_pair));
  fp->dim = X->num_dims;
  fp->dep = 0;
  fp->n = Y->dim;
  fp->slice = slice;
  fp->index = index;
  fp->x = X_data;
  fp->y = Y_data;
  fp->posx = 0;
  fp->posy = 0;
  fp->ofsx = calloc(fp->dim, sizeof(int));
  fp->ofsy = calloc(fp->dim, sizeof(int));
  fp->incx = calloc(fp->dim, sizeof(int));
  fp->incy = calloc(fp->dim, sizeof(int));
  c_slicing_offset(X, slice, fp->ofsx);
  c_slicing_stride(X, slice, fp->incx);
  c_ndarray_stride(Y, fp->incy);

  /** debug
  printf("slice: ");
  for (int i = 0; i < fp->dim; i++) {
    int64_t *j = fp->slice + (3 * i);
    printf("(%lld,", *(j + 0));
    printf("%lld,", *(j + 1));
    printf("%lld)", *(j + 2));
  }
  printf("\n");
  printf("index: ");
  for (int i = 0; i < 2; i++) {
    printf("%lld,", *(fp->index + i));
  }
  printf("\n");
   debug **/

  FUNCTION (c, fancy) (fp);

  free(fp->ofsx);
  free(fp->ofsy);
  free(fp->incx);
  free(fp->incy);
  free(fp);

  return Val_unit;
}


#endif /* OWL_ENABLE_TEMPLATE */
