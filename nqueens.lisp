(declaim (optimize (speed 3) (safety 0)))
(defun n-queens (n)
  (let ((try (make-array n :element-type '(unsigned-byte 32)))
        (hist (make-array (1+ n) :initial-element 0))
        (l 1)
        (col 0))
    (labels ((valid (try l)
               (loop :for i :upto l :do
                     (loop :for j :from (1+ i) :upto l :do
                           (let ((xi (aref try i))
                                 (xj (aref try j)))
                             (when (or (= xi xj) (= (abs (- xi xj)) (- j i)))
                               (return-from valid nil)))))
               t))

      (setf (aref try (1- l)) 1)
      (do () (nil)
       b2
        (incf (aref hist (1- l)))
        (if (> l n)
            (go b5)
            (progn
              (setf col 1)
              (setf (aref try (1- l)) col)))
       b3
        (when (valid try (1- l))
          (incf l)
          (go b2))
       b4
        (when (/= col n)
          (incf col)
          (setf (aref try (1- l)) col)
          (go b3))
       b5
        (decf l)
        (if (> l 0)
            (progn
              (setf col (aref try (1- l)))
              (go b4))
            (return hist))))))


(defun n-queens-w (n)
  (let ((try (make-array n :element-type 'fixnum))
        (nsol 0)
        (s (make-array n :element-type 'fixnum))
        (a (make-array n :element-type 'fixnum))
        (b (make-array n :element-type 'fixnum))
        (c (make-array n :element-type 'fixnum))
        (mask (1- (ash 1 n)))
        (l 0))
    (declare (type fixnum n l nsol mask) (optimize (speed 3) (safety 0)))
    (do () (nil)
     b2
      (when (>= l n)
        (progn
          (incf nsol)
          (go b4)))
      (setf (aref s l) (logand mask (lognot (logior (aref a l) (aref b l) (aref c l)))))
     b3
      (when (/= (aref s l) 0)
        (let ((min (logand (aref s l) (- (aref s l)))))
          (declare (type fixnum min))
          (setf (aref try l) (1- (integer-length min)))
          (when (< l (1- n))
            (setf (aref a (1+ l)) (logior (aref a l) min))
            (setf (aref b (1+ l)) (ash (logior (aref b l) min) -1))
            (setf (aref c (1+ l)) (ash (logior (aref c l) min) 1)))
          (incf l)
          (go b2)))
     b4
      (decf l)
      (if (>= l 0)
          (progn
            (setf (aref s l) (logxor (aref s l) (the fixnum (ash 1 (the fixnum (aref try l))))))
            (go b3))
          (return nsol)))))

(time (print (n-queens-w 16)))
