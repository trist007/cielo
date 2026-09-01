((nil . ((eval . (progn
  (let ((root (locate-dominating-file default-directory "build.bat")))
    (when (stringp root)
      (setq-local default-directory root)))
  (define-minor-mode my-terrain-project-mode
    "Local keybindings for this project directory."
    :lighter ""
    :keymap (let ((map (make-sparse-keymap)))
              (define-key map (kbd "M-m")
                (lambda () (interactive) (compile "build.bat")))
              (define-key map (kbd "<f2>")
                (lambda () (interactive)
                  (start-process "terrain" nil
                                 (expand-file-name "terrain.exe" default-directory))))
              (define-key map (kbd "<f3>")
                (lambda () (interactive)
                  (start-process "raddbg" nil "raddbg.exe"
                                 (expand-file-name "terrain.exe" default-directory))))
              map))
  (my-terrain-project-mode 1))))))
