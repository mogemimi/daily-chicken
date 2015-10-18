(use
  srfi-13
  vector-lib
  http-client
  openssl
  json)

;; (print (concat-query "fix typo language:cplusplus"))
;; ;=> fix+typo+language:cplusplus
(define concat-query
  (lambda (query)
    (assert (string? query))
    (string-join (string-tokenize query) "+")))

;; (print (github/search/issues "fix typo language:cplusplus"))
;; ;=> https://api.github.com/search/issues?q=fix+typo+language:cplusplus
(define github/search/issues
  (lambda (search-items)
    (assert (string? search-items))
    (string-concatenate (list
      "https://api.github.com"
      "/search/issues"
      "?q="
      (concat-query search-items)))))

;; (define items2 '#(("a" 42) ("b" 3) ("c" 17)))
;; (print (vector-find (lambda (x) (equal? (car x) "a")) items))
;; ;=> ("a" 42)
(use vector-lib)
(define (vector-find pred? . vecs)
  (cond
    ((apply vector-index pred? vecs) =>
      (lambda (index)
        (apply values (map (lambda (v) (vector-ref v index)) vecs))))
    (else #f)))
; In other implementation:
;(define vector-find (lambda (key items)
;  (assert (vector? items))
;  (vector-ref items (vector-index (lambda (item) (equal? (car item) key)) items))))

;; (define items '#(("a" . 42) ("b" . 3) ("c" . 17)))
;; (print (json-get-value "a" items))
;; ;=> 42
(define (json-get-value key object)
  (cdr (vector-find (lambda (x) (equal? (car x) key)) object)))

(let* (
  (uri (github/search/issues "fix typo language:cplusplus type:pr is:merged"))
  (response (with-input-from-request uri #f json-read)))
  (begin
    (with-output-to-file "output.scm"
      (lambda () (pretty-print response)))
    (newline)
    (print (json-get-value "total_count" response))
    (print (json-get-value "incomplete_results" response))
    (for-each
      (lambda (item) (print (json-get-value "url" item)))
      (json-get-value "items" response))
    (print "Done")))
