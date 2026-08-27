((nil . ((eval . (progn
  (let ((root (locate-dominating-file default-directory "build.bat")))
    (when (stringp root)
      (setq-local default-directory root)))

  (define-minor-mode my-terrain-project-mode
    "Local keybindings for this project directory."
    :lighter ""
    :keymap (let ((map (make-sparse-keymap)))
              (define-key map (kbd "M-m")
                (lambda ()
                  (interactive)
                    (compile "build.bat")))
              (define-key map (kbd "<f2>")
                (lambda ()
                  (interactive)
                    (start-process "terrain" nil
                                   (expand-file-name "terrain.exe" default-directory))))
              (define-key map (kbd "<f3>")
                (lambda ()
                  (interactive)
                    (start-process "raddbg" nil
                                   "raddbg.exe"
                                   (expand-file-name "terrain.exe" default-directory))))

              map))
  (my-terrain-project-mode 1)))))
 )

(defun +my/allman-braces ()
  (c-set-offset 'substatement-open 0)
  (c-set-offset 'inline-open 0)
  (c-set-offset 'block-open 0)
  (c-set-offset 'brace-list-open 0)
  (setq-local c-hanging-braces-alist
        '((substatement-open before after)
          (brace-list-open before after)
          (block-open before after)
          (defun-open before after)
          (class-open before after)
          (inline-open before after))))

(add-hook 'c-mode-common-hook #'+my/allman-braces)
