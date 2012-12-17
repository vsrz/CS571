;; cs571 Yoshii - recursion example in Lisp

;; interleaves x and y

(defun interleave (x y)
  (cond ((null x) y)
        ((null y) x)
        (t (cons (car x) (cons (car y) (interleave (cdr x) (cdr y)))))))

(print(interleave '(1 2 3 4) '(a b)))
(print(interleave '(1 2) '(a b c d)))


