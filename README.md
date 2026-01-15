# Share-Y 📸

Un gestionnaire de captures d'écran léger pour Linux, inspiré de ShareX pour Windows.

## ✨ Fonctionnalités

- **Capture d'écran**
  - Plein écran
  - Sélection de région
  - Fenêtre active

- **Stockage en mémoire**
  - Captures stockées en RAM (pas d'écriture sur disque)
  - Limite mémoire configurable (500 MB par défaut)
  - Dashboard avec vignettes

- **Intégration presse-papiers**
  - Double-clic pour copier
  - Menu contextuel rapide

- **System Tray**
  - Tourne en arrière-plan
  - Raccourcis globaux

## 🔧 Installation

```bash
# Installer les dépendances Python
pip install -r requirements.txt

# Ou avec pip3
pip3 install PyQt6 pynput Pillow
```

## 🚀 Utilisation

```bash
# Lancer l'application
python main.py
```

### Raccourcis clavier

| Raccourci | Action |
|-----------|--------|
| `Ctrl+Alt+P` | Capture plein écran |
| `Ctrl+Alt+R` | Capture région |
| `Escape` | Cacher la fenêtre |

### Dashboard

- **Simple clic** - Sélectionner une capture
- **Double clic** - Copier vers le presse-papiers
- **Clic droit** - Menu contextuel (copier, sauvegarder, supprimer)

## 📁 Structure du projet

```
Share-Y/
├── main.py              # Point d'entrée
├── requirements.txt     # Dépendances
├── src/
│   ├── core/
│   │   ├── screenshot.py   # Modèle Screenshot
│   │   └── buffer.py       # Stockage en mémoire
│   ├── capture/
│   │   └── capturer.py     # Capture via outils système
│   ├── ui/
│   │   ├── main_window.py  # Fenêtre principale
│   │   ├── thumbnail.py    # Widget vignette
│   │   └── flow_layout.py  # Layout fluide
│   └── hotkeys/
│       └── manager.py      # Gestionnaire de raccourcis
└── resources/
    └── icons/
```

