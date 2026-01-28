// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "WordStyleDlg.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QSplitter>
#include <QtGui/QPainter>
#include <QtGui/QFontDatabase>

namespace QtControls {

// ============================================================================
// WordStyleDlg implementation
// ============================================================================

WordStyleDlg::WordStyleDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Style Configurator"));
    resize(800, 600);
}

WordStyleDlg::~WordStyleDlg() {
    // Cleanup is handled by parent class
}

void WordStyleDlg::init() {
    setupUI();
    connectSignals();
    loadLanguages();
    loadThemes();
    loadFontSizes();
    updateGlobalOverrideCtrls();
    setVisualFromStyleList();
}

void WordStyleDlg::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Left panel - Language and Style selection
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QGridLayout(leftPanel);
    leftLayout->setSpacing(8);
    leftLayout->setContentsMargins(8, 8, 8, 8);

    int row = 0;
    createLanguageSection(leftLayout, row);
    createStyleSection(leftLayout, row);

    leftPanel->setLayout(leftLayout);
    splitter->addWidget(leftPanel);

    // Right panel - Style settings
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QGridLayout(rightPanel);
    rightLayout->setSpacing(8);
    rightLayout->setContentsMargins(8, 8, 8, 8);

    row = 0;
    createFontSection(rightLayout, row);
    createColorSection(rightLayout, row);
    createGlobalOverrideSection(rightLayout, row);
    createPreviewSection(rightLayout, row);

    rightPanel->setLayout(rightLayout);
    splitter->addWidget(rightPanel);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);

    // Button section at bottom
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _saveThemeBtn = new QPushButton(tr("Save & Close"), this);
    buttonLayout->addWidget(_saveThemeBtn);

    _saveAsThemeBtn = new QPushButton(tr("Save As..."), this);
    buttonLayout->addWidget(_saveAsThemeBtn);

    _cancelBtn = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(_cancelBtn);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void WordStyleDlg::createLanguageSection(QGridLayout* layout, int& row) {
    // Theme selector
    layout->addWidget(new QLabel(tr("Select theme:"), this), row, 0);
    _themeCombo = new QComboBox(this);
    layout->addWidget(_themeCombo, row, 1, 1, 2);
    row++;

    // Language selector
    layout->addWidget(new QLabel(tr("Language:"), this), row, 0);
    _languageCombo = new QComboBox(this);
    _languageCombo->setMinimumWidth(200);
    layout->addWidget(_languageCombo, row, 1, 1, 2);
    row++;
}

void WordStyleDlg::createStyleSection(QGridLayout* layout, int& row) {
    layout->addWidget(new QLabel(tr("Style:"), this), row, 0, Qt::AlignTop);

    _styleList = new QListWidget(this);
    _styleList->setMinimumHeight(200);
    layout->addWidget(_styleList, row, 1, 1, 2);
    row++;

    // Style description
    _styleDescLabel = new QLabel(this);
    _styleDescLabel->setWordWrap(true);
    _styleDescLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    layout->addWidget(_styleDescLabel, row, 1, 1, 2);
    row++;

    // User ext and keywords
    layout->addWidget(new QLabel(tr("User ext.:"), this), row, 0);
    _userExtEdit = new QLineEdit(this);
    layout->addWidget(_userExtEdit, row, 1, 1, 2);
    row++;

    layout->addWidget(new QLabel(tr("User keywords:"), this), row, 0);
    _userKeywordsEdit = new QLineEdit(this);
    layout->addWidget(_userKeywordsEdit, row, 1, 1, 2);
    row++;
}

void WordStyleDlg::createFontSection(QGridLayout* layout, int& row) {
    auto* fontGroup = new QGroupBox(tr("Font Style"), this);
    auto* fontLayout = new QGridLayout(fontGroup);

    // Font name
    fontLayout->addWidget(new QLabel(tr("Font name:"), this), 0, 0);
    _fontCombo = new QFontComboBox(this);
    fontLayout->addWidget(_fontCombo, 0, 1, 1, 2);

    // Font size
    fontLayout->addWidget(new QLabel(tr("Font size:"), this), 1, 0);
    _fontSizeCombo = new QComboBox(this);
    _fontSizeCombo->setEditable(true);
    fontLayout->addWidget(_fontSizeCombo, 1, 1);

    // Font style checkboxes
    _boldCheck = new QCheckBox(tr("Bold"), this);
    fontLayout->addWidget(_boldCheck, 1, 2);

    _italicCheck = new QCheckBox(tr("Italic"), this);
    fontLayout->addWidget(_italicCheck, 2, 1);

    _underlineCheck = new QCheckBox(tr("Underline"), this);
    fontLayout->addWidget(_underlineCheck, 2, 2);

    layout->addWidget(fontGroup, row, 0, 1, 3);
    row++;
}

void WordStyleDlg::createColorSection(QGridLayout* layout, int& row) {
    auto* colorGroup = new QGroupBox(tr("Colors"), this);
    auto* colorLayout = new QGridLayout(colorGroup);

    // Foreground color
    colorLayout->addWidget(new QLabel(tr("Foreground color:"), this), 0, 0);
    _fgColorBtn = new QPushButton(this);
    _fgColorBtn->setFixedSize(40, 24);
    colorLayout->addWidget(_fgColorBtn, 0, 1);
    _fgColorLabel = new QLabel(tr("Click to change"), this);
    colorLayout->addWidget(_fgColorLabel, 0, 2);

    // Background color
    colorLayout->addWidget(new QLabel(tr("Background color:"), this), 1, 0);
    _bgColorBtn = new QPushButton(this);
    _bgColorBtn->setFixedSize(40, 24);
    colorLayout->addWidget(_bgColorBtn, 1, 1);
    _bgColorLabel = new QLabel(tr("Click to change"), this);
    colorLayout->addWidget(_bgColorLabel, 1, 2);

    layout->addWidget(colorGroup, row, 0, 1, 3);
    row++;
}

void WordStyleDlg::createGlobalOverrideSection(QGridLayout* layout, int& row) {
    _globalOverrideGroup = new QGroupBox(tr("Global Override"), this);
    auto* overrideLayout = new QGridLayout(_globalOverrideGroup);

    _globalFgCheck = new QCheckBox(tr("Foreground color"), this);
    overrideLayout->addWidget(_globalFgCheck, 0, 0);

    _globalBgCheck = new QCheckBox(tr("Background color"), this);
    overrideLayout->addWidget(_globalBgCheck, 0, 1);

    _globalFontCheck = new QCheckBox(tr("Font name"), this);
    overrideLayout->addWidget(_globalFontCheck, 1, 0);

    _globalFontSizeCheck = new QCheckBox(tr("Font size"), this);
    overrideLayout->addWidget(_globalFontSizeCheck, 1, 1);

    _globalBoldCheck = new QCheckBox(tr("Bold"), this);
    overrideLayout->addWidget(_globalBoldCheck, 2, 0);

    _globalItalicCheck = new QCheckBox(tr("Italic"), this);
    overrideLayout->addWidget(_globalItalicCheck, 2, 1);

    _globalUnderlineCheck = new QCheckBox(tr("Underline"), this);
    overrideLayout->addWidget(_globalUnderlineCheck, 3, 0);

    _goToSettingsBtn = new QPushButton(tr("Go to Settings"), this);
    overrideLayout->addWidget(_goToSettingsBtn, 3, 1);

    layout->addWidget(_globalOverrideGroup, row, 0, 1, 3);
    row++;
}

void WordStyleDlg::createPreviewSection(QGridLayout* layout, int& row) {
    auto* previewGroup = new QGroupBox(tr("Preview"), this);
    auto* previewLayout = new QVBoxLayout(previewGroup);

    _preview = new ScintillaPreview(this);
    _preview->setMinimumHeight(150);
    previewLayout->addWidget(_preview);

    layout->addWidget(previewGroup, row, 0, 1, 3);
    row++;
}

void WordStyleDlg::connectSignals() {
    // Language and style selection
    connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WordStyleDlg::onLanguageChanged);
    connect(_styleList, &QListWidget::currentRowChanged,
            this, &WordStyleDlg::onStyleChanged);

    // Font settings
    connect(_fontCombo, &QFontComboBox::currentTextChanged,
            this, &WordStyleDlg::onFontChanged);
    connect(_fontSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { onFontSizeChanged(_fontSizeCombo->currentText()); });

    // Font style
    connect(_boldCheck, &QCheckBox::toggled, this, &WordStyleDlg::onBoldToggled);
    connect(_italicCheck, &QCheckBox::toggled, this, &WordStyleDlg::onItalicToggled);
    connect(_underlineCheck, &QCheckBox::toggled, this, &WordStyleDlg::onUnderlineToggled);

    // Color pickers
    connect(_fgColorBtn, &QPushButton::clicked, this, &WordStyleDlg::onFgColorClicked);
    connect(_bgColorBtn, &QPushButton::clicked, this, &WordStyleDlg::onBgColorClicked);

    // Theme management
    connect(_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WordStyleDlg::onThemeChanged);
    connect(_saveThemeBtn, &QPushButton::clicked, this, &WordStyleDlg::onSaveThemeClicked);
    connect(_saveAsThemeBtn, &QPushButton::clicked, this, &WordStyleDlg::onSaveAsThemeClicked);

    // Global override
    connect(_globalFgCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFgToggled);
    connect(_globalBgCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalBgToggled);
    connect(_globalFontCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFontToggled);
    connect(_globalFontSizeCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFontSizeToggled);
    connect(_globalBoldCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalBoldToggled);
    connect(_globalItalicCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalItalicToggled);
    connect(_globalUnderlineCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalUnderlineToggled);

    // Navigation
    connect(_goToSettingsBtn, &QPushButton::clicked, this, &WordStyleDlg::onGoToSettingsClicked);

    // Dialog actions
    connect(_cancelBtn, &QPushButton::clicked, this, &WordStyleDlg::onCancelClicked);

    // User keywords and extensions
    connect(_userKeywordsEdit, &QLineEdit::textChanged, this, &WordStyleDlg::onUserKeywordsChanged);
    connect(_userExtEdit, &QLineEdit::textChanged, this, &WordStyleDlg::onUserExtChanged);
}

void WordStyleDlg::loadLanguages() {
    // Get styles from NppParameters
    // Note: In the actual implementation, this would use NppParameters::getInstance()
    // For now, we populate with sample data

    _languageCombo->clear();
    _languageCombo->addItem(tr("Global Styles"));

    // Add all lexers
    // In real implementation, iterate through _lsArray
    // for (size_t i = 0; i < _lsArray.getNbLexer(); ++i) {
    //     _languageCombo->addItem(QString::fromWCharArray(_lsArray.getLexerDescFromIndex(i)));
    // }

    _languageCombo->setCurrentIndex(0);
    setStyleListFromLexer(0);
}

void WordStyleDlg::loadThemes() {
    _themeCombo->clear();

    // In real implementation:
    // ThemeSwitcher& themeSwitcher = NppParameters::getInstance().getThemeSwitcher();
    // for (size_t i = 0; i < themeSwitcher.size(); ++i) {
    //     auto& themeInfo = themeSwitcher.getElementFromIndex(i);
    //     _themeCombo->addItem(QString::fromWCharArray(themeInfo.first.c_str()));
    // }

    _themeCombo->addItem(tr("Default (stylers.xml)"));
    _themeCombo->addItem(tr("Dark Mode"));
    _themeCombo->addItem(tr("Obsidian"));
    _themeCombo->addItem(tr("Zenburn"));

    _currentThemeIndex = 0;
    _themeCombo->setCurrentIndex(_currentThemeIndex);
}

void WordStyleDlg::loadFontSizes() {
    _fontSizeCombo->clear();
    const char* fontSizeStrs[] = {
        "5", "6", "7", "8", "9", "10", "11", "12", "14", "16",
        "18", "20", "22", "24", "26", "28", "36", "48", "72"
    };
    for (const auto* size : fontSizeStrs) {
        _fontSizeCombo->addItem(QString::fromUtf8(size));
    }
}

void WordStyleDlg::setStyleListFromLexer(int lexerIndex) {
    _currentLexerIndex = lexerIndex;
    _styleList->clear();

    // In real implementation:
    // if (lexerIndex == 0) {
    //     // Global styles
    //     for (const auto& style : _globalStyles) {
    //         _styleList->addItem(QString::fromWCharArray(style._styleDesc.c_str()));
    //     }
    // } else {
    //     LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(lexerIndex - 1);
    //     for (const auto& style : lexerStyler) {
    //         _styleList->addItem(QString::fromWCharArray(style._styleDesc.c_str()));
    //     }
    // }

    // Sample data for now
    if (lexerIndex == 0) {
        _styleList->addItem(tr("Global override"));
        _styleList->addItem(tr("Default Style"));
        _styleList->addItem(tr("Indent guideline style"));
        _styleList->addItem(tr("Brace highlight style"));
        _styleList->addItem(tr("Bad brace colour"));
        _styleList->addItem(tr("Current line background colour"));
        _styleList->addItem(tr("Selected text colour"));
        _styleList->addItem(tr("Caret colour"));
        _styleList->addItem(tr("Edge colour"));
        _styleList->addItem(tr("Line number margin"));
        _styleList->addItem(tr("Fold"));
        _styleList->addItem(tr("Fold active"));
        _styleList->addItem(tr("Fold margin"));
    } else {
        _styleList->addItem(tr("DEFAULT"));
        _styleList->addItem(tr("COMMENT"));
        _styleList->addItem(tr("COMMENT LINE"));
        _styleList->addItem(tr("NUMBER"));
        _styleList->addItem(tr("WORD"));
        _styleList->addItem(tr("STRING"));
        _styleList->addItem(tr("CHARACTER"));
        _styleList->addItem(tr("OPERATOR"));
        _styleList->addItem(tr("PREPROCESSOR"));
    }

    if (_styleList->count() > 0) {
        _styleList->setCurrentRow(0);
        setVisualFromStyleList();
    }
}

void WordStyleDlg::setVisualFromStyleList() {
    // Get current style and update UI controls
    // Style& style = getCurrentStyler();

    // Update font controls
    // _fontCombo->setCurrentFont(QFont(QString::fromWCharArray(style._fontName.c_str())));
    // _fontSizeCombo->setCurrentText(QString::number(style._fontSize));
    // _boldCheck->setChecked(style._fontStyle & FONTSTYLE_BOLD);
    // _italicCheck->setChecked(style._fontStyle & FONTSTYLE_ITALIC);
    // _underlineCheck->setChecked(style._fontStyle & FONTSTYLE_UNDERLINE);

    // Update color buttons
    // QColor fgColor = colorFromColorRef(style._fgColor);
    // QColor bgColor = colorFromColorRef(style._bgColor);
    // _fgColorBtn->setStyleSheet(QString("background-color: %1").arg(fgColor.name()));
    // _bgColorBtn->setStyleSheet(QString("background-color: %1").arg(bgColor.name()));

    // Update preview
    updatePreview();
}

void WordStyleDlg::doDialog() {
    goToCenter();
    display(true);
}

void WordStyleDlg::prepare2Cancel() {
    if (_isDirty) {
        // Restore original values
        if (_restoreInvalid) {
            // Reload stylers from file
            // NppParameters::getInstance().reloadStylers(_themeName.toStdWString().c_str());
        }

        // Restore backup values
        // _lsArray = _styles2restored;
        // _globalStyles = _gstyles2restored;

        restoreGlobalOverrideValues();

        _restoreInvalid = false;
        _isDirty = false;
        _isThemeDirty = false;
        _isThemeChanged = false;
    }
}

bool WordStyleDlg::selectThemeByName(const QString& themeName) {
    // Find theme in combo
    for (int i = 0; i < _themeCombo->count(); ++i) {
        if (_themeCombo->itemText(i) == themeName) {
            _themeCombo->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

bool WordStyleDlg::goToSection(const QString& section) {
    // Parse "Language:Style" format
    int colonPos = section.indexOf(':');
    if (colonPos == -1) return false;

    QString language = section.left(colonPos);
    QString styleName = section.mid(colonPos + 1);

    // Find and select language
    for (int i = 0; i < _languageCombo->count(); ++i) {
        if (_languageCombo->itemText(i) == language) {
            _languageCombo->setCurrentIndex(i);

            // Find and select style
            for (int j = 0; j < _styleList->count(); ++j) {
                if (_styleList->item(j)->text() == styleName) {
                    _styleList->setCurrentRow(j);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void WordStyleDlg::restoreGlobalOverrideValues() {
    // Restore global override from backup
    // NppParameters::getInstance().getGlobalOverrideStyle() = _gOverride2restored;
}

// Slot implementations
void WordStyleDlg::onLanguageChanged(int index) {
    if (index >= 0) {
        bool prevThemeState = _isThemeDirty;
        setStyleListFromLexer(index);
        _isThemeDirty = prevThemeState;
    }
}

void WordStyleDlg::onStyleChanged(int index) {
    if (index >= 0) {
        setVisualFromStyleList();
    }
}

void WordStyleDlg::onFontChanged(const QString& font) {
    (void)font;
    updateFontName();
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onFontSizeChanged(const QString& size) {
    (void)size;
    updateFontSize();
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onBoldToggled(bool checked) {
    (void)checked;
    updateFontStyleStatus(BOLD_STATUS);
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onItalicToggled(bool checked) {
    (void)checked;
    updateFontStyleStatus(ITALIC_STATUS);
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onUnderlineToggled(bool checked) {
    (void)checked;
    updateFontStyleStatus(UNDERLINE_STATUS);
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onFgColorClicked() {
    QColor color = QColorDialog::getColor(Qt::black, this, tr("Select Foreground Color"));
    if (color.isValid()) {
        _fgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        updateColour(C_FOREGROUND);
        notifyDataModified();
        applyStyleChanges();
    }
}

void WordStyleDlg::onBgColorClicked() {
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Background Color"));
    if (color.isValid()) {
        _bgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        updateColour(C_BACKGROUND);
        notifyDataModified();
        applyStyleChanges();
    }
}

void WordStyleDlg::onThemeChanged(int index) {
    if (index >= 0 && index != _currentThemeIndex) {
        applyCurrentSelectedThemeAndUpdateUI();
    }
}

void WordStyleDlg::onSaveThemeClicked() {
    if (_isDirty) {
        // Save current styles
        // QString savePath = NppParameters::getInstance().writeStyles(_lsArray, _globalStyles);
        // if (!savePath.isEmpty()) {
        //     updateThemeName(savePath);
        // }

        _isDirty = false;
        _isThemeDirty = false;
        _restoreInvalid = false;
    }
    display(false);
}

void WordStyleDlg::onSaveAsThemeClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Theme As"),
                                                    QString(), tr("XML files (*.xml)"));
    if (!fileName.isEmpty()) {
        // Save theme to file
        // TODO: Implement theme saving
        updateThemeName(fileName);
    }
}

void WordStyleDlg::onGlobalFgToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableFg = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalBgToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableBg = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalFontToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableFont = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalFontSizeToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableFontSize = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalBoldToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableBold = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalItalicToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableItalic = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGlobalUnderlineToggled(bool checked) {
    // NppParameters::getInstance().getGlobalOverrideStyle().enableUnderLine = checked;
    (void)checked;
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onGoToSettingsClicked() {
    // Emit signal or notify parent to open preferences
    // TODO: Implement navigation to preferences
}

void WordStyleDlg::onOkClicked() {
    onSaveThemeClicked();
}

void WordStyleDlg::onCancelClicked() {
    prepare2Cancel();
    display(false);
}

void WordStyleDlg::onUserKeywordsChanged() {
    updateUserKeywords();
    notifyDataModified();
    applyStyleChanges();
}

void WordStyleDlg::onUserExtChanged() {
    updateExtension();
    notifyDataModified();
    applyStyleChanges();
}

// Update methods
void WordStyleDlg::updateFontName() {
    // Style& style = getCurrentStyler();
    // style._fontName = _fontCombo->currentFont().family().toStdWString();
    // style._isFontEnabled = true;
}

void WordStyleDlg::updateFontSize() {
    // Style& style = getCurrentStyler();
    // style._fontSize = _fontSizeCombo->currentText().toInt();
}

void WordStyleDlg::updateFontStyleStatus(fontStyleType styleType) {
    // Style& style = getCurrentStyler();
    // int fontStyle = style._fontStyle;

    // switch (styleType) {
    //     case BOLD_STATUS:
    //         if (_boldCheck->isChecked()) fontStyle |= FONTSTYLE_BOLD;
    //         else fontStyle &= ~FONTSTYLE_BOLD;
    //         break;
    //     case ITALIC_STATUS:
    //         if (_italicCheck->isChecked()) fontStyle |= FONTSTYLE_ITALIC;
    //         else fontStyle &= ~FONTSTYLE_ITALIC;
    //         break;
    //     case UNDERLINE_STATUS:
    //         if (_underlineCheck->isChecked()) fontStyle |= FONTSTYLE_UNDERLINE;
    //         else fontStyle &= ~FONTSTYLE_UNDERLINE;
    //         break;
    // }
    // style._fontStyle = fontStyle;
    (void)styleType;
}

void WordStyleDlg::updateColour(bool isBackground) {
    // Style& style = getCurrentStyler();
    // if (isBackground) {
    //     style._bgColor = colorRefFromColor(QColor(_bgColorBtn->styleSheet().mid(17)));
    // } else {
    //     style._fgColor = colorRefFromColor(QColor(_fgColorBtn->styleSheet().mid(17)));
    // }
    (void)isBackground;
}

void WordStyleDlg::updateUserKeywords() {
    // Update user keywords for current lexer
}

void WordStyleDlg::updateExtension() {
    // Update file extensions for current lexer
}

void WordStyleDlg::updateGlobalOverrideCtrls() {
    // Update global override checkboxes from NppParameters
    // const GlobalOverride& glo = NppParameters::getInstance().getGlobalOverrideStyle();
    // _globalFgCheck->setChecked(glo.enableFg);
    // _globalBgCheck->setChecked(glo.enableBg);
    // _globalFontCheck->setChecked(glo.enableFont);
    // _globalFontSizeCheck->setChecked(glo.enableFontSize);
    // _globalBoldCheck->setChecked(glo.enableBold);
    // _globalItalicCheck->setChecked(glo.enableItalic);
    // _globalUnderlineCheck->setChecked(glo.enableUnderLine);
}

void WordStyleDlg::applyStyleChanges() {
    // Apply changes to preview and notify parent
    updatePreview();
    // emit stylesChanged();
}

void WordStyleDlg::updatePreview() {
    if (_preview) {
        // Apply current style to preview
        // Style& style = getCurrentStyler();
        // _preview->applyStyle(style);
    }
}

void WordStyleDlg::notifyDataModified() {
    _isDirty = true;
    _saveThemeBtn->setEnabled(true);
}

void WordStyleDlg::switchToTheme(int themeIndex) {
    // Load theme from file
    // ThemeSwitcher& themeSwitcher = NppParameters::getInstance().getThemeSwitcher();
    // auto& themeInfo = themeSwitcher.getElementFromIndex(themeIndex);
    // NppParameters::getInstance().reloadStylers(themeInfo.second.c_str());

    // Reload UI
    loadLanguages();
    setVisualFromStyleList();

    _currentThemeIndex = themeIndex;
    _isThemeChanged = true;
    _isThemeDirty = true;
}

void WordStyleDlg::updateThemeName(const QString& themeName) {
    _themeName = themeName;
    // NppParameters::getInstance().getNppGUI()._themeName = themeName.toStdWString();
}

void WordStyleDlg::applyCurrentSelectedThemeAndUpdateUI() {
    int newThemeIndex = _themeCombo->currentIndex();
    if (newThemeIndex != _currentThemeIndex) {
        switchToTheme(newThemeIndex);
        notifyDataModified();
    }
}

// Utility methods
Style& WordStyleDlg::getCurrentStyler() {
    // if (_currentLexerIndex == 0) {
    //     int styleIndex = _styleList->currentRow();
    //     return _globalStyles.getStyler(styleIndex);
    // } else {
    //     LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(_currentLexerIndex - 1);
    //     int styleIndex = _styleList->currentRow();
    //     return lexerStyler.getStyler(styleIndex);
    // }
    static Style dummyStyle;
    return dummyStyle;
}

const Style& WordStyleDlg::getCurrentStyler() const {
    return const_cast<WordStyleDlg*>(this)->getCurrentStyler();
}

bool WordStyleDlg::getCurrentStyleName(QString& styleName) const {
    int row = _styleList->currentRow();
    if (row < 0) return false;
    styleName = _styleList->item(row)->text();
    return true;
}

int WordStyleDlg::getApplicationInfo() const {
    QString styleName;
    if (!getCurrentStyleName(styleName)) {
        return 0; // NO_VISUAL_CHANGE
    }

    if (styleName == tr("Default Style")) {
        return 1 | 2; // GENERAL_CHANGE | THEME_CHANGE
    }

    // Check for special styles that affect UI
    if (styleName.contains(tr("Mark Style")) ||
        styleName.contains(tr("Tab colour"))) {
        return 1 | 4; // GENERAL_CHANGE | COLOR_CHANGE_4_MENU
    }

    return 1; // GENERAL_CHANGE
}

int WordStyleDlg::whichTabColourIndex() const {
    QString styleName;
    if (!getCurrentStyleName(styleName)) return -1;

    // Check for tab color styles
    if (styleName == tr("Active tab text colour")) return 0;
    if (styleName == tr("Active tab background colour")) return 1;
    if (styleName == tr("Inactive tabs")) return 2;

    return -1;
}

int WordStyleDlg::whichIndividualTabColourId() const {
    QString styleName;
    if (!getCurrentStyleName(styleName)) return -1;

    // Check for individual tab color styles
    for (int i = 1; i <= 5; ++i) {
        if (styleName == tr("Tab colour %1").arg(i)) {
            return i - 1;
        }
    }

    return -1;
}

int WordStyleDlg::whichFindDlgStatusMsgColourIndex() const {
    QString styleName;
    if (!getCurrentStyleName(styleName)) return -1;

    // Check for find dialog status message styles
    if (styleName.contains(tr("Find status"))) {
        if (styleName.contains(tr("Not found"))) return 0;
        if (styleName.contains(tr("Message"))) return 1;
        if (styleName.contains(tr("Search end reached"))) return 2;
    }

    return -1;
}

bool WordStyleDlg::isDocumentMapStyle() const {
    QString styleName;
    if (!getCurrentStyleName(styleName)) return false;

    return (styleName == tr("Document Map"));
}

void WordStyleDlg::enableFontControls(bool enable) {
    _fontCombo->setEnabled(enable);
    _fontSizeCombo->setEnabled(enable);
    _boldCheck->setEnabled(enable);
    _italicCheck->setEnabled(enable);
    _underlineCheck->setEnabled(enable);
}

void WordStyleDlg::enableColorControls(bool enable) {
    _fgColorBtn->setEnabled(enable);
    _bgColorBtn->setEnabled(enable);
}

QColor WordStyleDlg::colorFromColorRef(COLORREF color) const {
    return QColor(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF WordStyleDlg::colorRefFromColor(const QColor& color) const {
    return RGB(color.red(), color.green(), color.blue());
}

// ============================================================================
// ScintillaPreview implementation
// ============================================================================

ScintillaPreview::ScintillaPreview(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(200, 100);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
}

ScintillaPreview::~ScintillaPreview() = default;

void ScintillaPreview::setSampleText(const QString& text) {
    _sampleText = text;
    update();
}

void ScintillaPreview::setLexer(int lexerId) {
    _lexerId = lexerId;
    update();
}

void ScintillaPreview::applyStyle(const Style& style) {
    (void)style;
    // Store and apply style
    update();
}

void ScintillaPreview::clearStyles() {
    _styles.clear();
    update();
}

void ScintillaPreview::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    // Draw sample text with syntax highlighting
    QFont font("Consolas", 10);
    painter.setFont(font);
    painter.setPen(Qt::black);

    QString previewText = _sampleText.isEmpty()
        ? tr("// Sample preview text\nfunction example() {\n    return 42;\n}")
        : _sampleText;

    int y = 20;
    for (const QString& line : previewText.split('\n')) {
        painter.drawText(10, y, line);
        y += painter.fontMetrics().height();
    }
}

} // namespace QtControls
