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
#include <QFile>

namespace QtControls {

// ============================================================================
// WordStyleDlg implementation
// ============================================================================

WordStyleDlg::WordStyleDlg(QWidget* parent)
	: StaticDialog(parent)
{
	setWindowTitle(tr("Style Configurator"));
	resize(800, 600);
}

WordStyleDlg::~WordStyleDlg()
{
	// Cleanup is handled by parent class
}

void WordStyleDlg::init()
{
	setupUI();
	connectSignals();

	// Load style data from NppParameters
	NppParameters& nppParams = NppParameters::getInstance();
	_lsArray = nppParams.getLStylerArray();
	_globalStyles = nppParams.getGlobalStylers();

	// Keep backup for cancel
	_styles2restored = _lsArray;
	_gstyles2restored = _globalStyles;
	_gOverride2restored = nppParams.getGlobalOverrideStyle();

	_themeName = QString::fromWCharArray(nppParams.getNppGUI()._themeName.c_str());

	loadThemes();
	loadLanguages();
	loadFontSizes();
	updateGlobalOverrideCtrls();
	setVisualFromStyleList();
}

void WordStyleDlg::setupUI()
{
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

void WordStyleDlg::createLanguageSection(QGridLayout* layout, int& row)
{
	layout->addWidget(new QLabel(tr("Select theme:"), this), row, 0);
	_themeCombo = new QComboBox(this);
	layout->addWidget(_themeCombo, row, 1, 1, 2);
	row++;

	layout->addWidget(new QLabel(tr("Language:"), this), row, 0);
	_languageCombo = new QComboBox(this);
	_languageCombo->setMinimumWidth(200);
	layout->addWidget(_languageCombo, row, 1, 1, 2);
	row++;
}

void WordStyleDlg::createStyleSection(QGridLayout* layout, int& row)
{
	layout->addWidget(new QLabel(tr("Style:"), this), row, 0, Qt::AlignTop);

	_styleList = new QListWidget(this);
	_styleList->setMinimumHeight(200);
	layout->addWidget(_styleList, row, 1, 1, 2);
	row++;

	_styleDescLabel = new QLabel(this);
	_styleDescLabel->setWordWrap(true);
	_styleDescLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(_styleDescLabel, row, 1, 1, 2);
	row++;

	layout->addWidget(new QLabel(tr("User ext.:"), this), row, 0);
	_userExtEdit = new QLineEdit(this);
	layout->addWidget(_userExtEdit, row, 1, 1, 2);
	row++;

	layout->addWidget(new QLabel(tr("User keywords:"), this), row, 0);
	_userKeywordsEdit = new QLineEdit(this);
	layout->addWidget(_userKeywordsEdit, row, 1, 1, 2);
	row++;
}

void WordStyleDlg::createFontSection(QGridLayout* layout, int& row)
{
	auto* fontGroup = new QGroupBox(tr("Font Style"), this);
	auto* fontLayout = new QGridLayout(fontGroup);

	fontLayout->addWidget(new QLabel(tr("Font name:"), this), 0, 0);
	_fontCombo = new QFontComboBox(this);
	fontLayout->addWidget(_fontCombo, 0, 1, 1, 2);

	fontLayout->addWidget(new QLabel(tr("Font size:"), this), 1, 0);
	_fontSizeCombo = new QComboBox(this);
	_fontSizeCombo->setEditable(true);
	fontLayout->addWidget(_fontSizeCombo, 1, 1);

	_boldCheck = new QCheckBox(tr("Bold"), this);
	fontLayout->addWidget(_boldCheck, 1, 2);

	_italicCheck = new QCheckBox(tr("Italic"), this);
	fontLayout->addWidget(_italicCheck, 2, 1);

	_underlineCheck = new QCheckBox(tr("Underline"), this);
	fontLayout->addWidget(_underlineCheck, 2, 2);

	layout->addWidget(fontGroup, row, 0, 1, 3);
	row++;
}

void WordStyleDlg::createColorSection(QGridLayout* layout, int& row)
{
	auto* colorGroup = new QGroupBox(tr("Colors"), this);
	auto* colorLayout = new QGridLayout(colorGroup);

	colorLayout->addWidget(new QLabel(tr("Foreground color:"), this), 0, 0);
	_fgColorBtn = new QPushButton(this);
	_fgColorBtn->setFixedSize(40, 24);
	colorLayout->addWidget(_fgColorBtn, 0, 1);
	_fgColorLabel = new QLabel(tr("Click to change"), this);
	colorLayout->addWidget(_fgColorLabel, 0, 2);

	colorLayout->addWidget(new QLabel(tr("Background color:"), this), 1, 0);
	_bgColorBtn = new QPushButton(this);
	_bgColorBtn->setFixedSize(40, 24);
	colorLayout->addWidget(_bgColorBtn, 1, 1);
	_bgColorLabel = new QLabel(tr("Click to change"), this);
	colorLayout->addWidget(_bgColorLabel, 1, 2);

	layout->addWidget(colorGroup, row, 0, 1, 3);
	row++;
}

void WordStyleDlg::createGlobalOverrideSection(QGridLayout* layout, int& row)
{
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

void WordStyleDlg::createPreviewSection(QGridLayout* layout, int& row)
{
	auto* previewGroup = new QGroupBox(tr("Preview"), this);
	auto* previewLayout = new QVBoxLayout(previewGroup);

	_preview = new ScintillaPreview(this);
	_preview->setMinimumHeight(150);
	previewLayout->addWidget(_preview);

	layout->addWidget(previewGroup, row, 0, 1, 3);
	row++;
}

void WordStyleDlg::connectSignals()
{
	connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &WordStyleDlg::onLanguageChanged);
	connect(_styleList, &QListWidget::currentRowChanged,
			this, &WordStyleDlg::onStyleChanged);

	connect(_fontCombo, &QFontComboBox::currentTextChanged,
			this, &WordStyleDlg::onFontChanged);
	connect(_fontSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, [this](int) { onFontSizeChanged(_fontSizeCombo->currentText()); });

	connect(_boldCheck, &QCheckBox::toggled, this, &WordStyleDlg::onBoldToggled);
	connect(_italicCheck, &QCheckBox::toggled, this, &WordStyleDlg::onItalicToggled);
	connect(_underlineCheck, &QCheckBox::toggled, this, &WordStyleDlg::onUnderlineToggled);

	connect(_fgColorBtn, &QPushButton::clicked, this, &WordStyleDlg::onFgColorClicked);
	connect(_bgColorBtn, &QPushButton::clicked, this, &WordStyleDlg::onBgColorClicked);

	connect(_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &WordStyleDlg::onThemeChanged);
	connect(_saveThemeBtn, &QPushButton::clicked, this, &WordStyleDlg::onSaveThemeClicked);
	connect(_saveAsThemeBtn, &QPushButton::clicked, this, &WordStyleDlg::onSaveAsThemeClicked);

	connect(_globalFgCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFgToggled);
	connect(_globalBgCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalBgToggled);
	connect(_globalFontCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFontToggled);
	connect(_globalFontSizeCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalFontSizeToggled);
	connect(_globalBoldCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalBoldToggled);
	connect(_globalItalicCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalItalicToggled);
	connect(_globalUnderlineCheck, &QCheckBox::toggled, this, &WordStyleDlg::onGlobalUnderlineToggled);

	connect(_goToSettingsBtn, &QPushButton::clicked, this, &WordStyleDlg::onGoToSettingsClicked);
	connect(_cancelBtn, &QPushButton::clicked, this, &WordStyleDlg::onCancelClicked);

	connect(_userKeywordsEdit, &QLineEdit::textChanged, this, &WordStyleDlg::onUserKeywordsChanged);
	connect(_userExtEdit, &QLineEdit::textChanged, this, &WordStyleDlg::onUserExtChanged);
}

void WordStyleDlg::loadLanguages()
{
	_languageCombo->clear();
	_languageCombo->addItem(tr("Global Styles"));

	for (size_t i = 0; i < _lsArray.getNbLexer(); ++i)
	{
		_languageCombo->addItem(QString::fromWCharArray(_lsArray.getLexerDescFromIndex(i)));
	}

	_languageCombo->setCurrentIndex(0);
	setStyleListFromLexer(0);
}

void WordStyleDlg::loadThemes()
{
	_themeCombo->clear();

	ThemeSwitcher& themeSwitcher = NppParameters::getInstance().getThemeSwitcher();
	for (size_t i = 0; i < themeSwitcher.size(); ++i)
	{
		auto& themeInfo = themeSwitcher.getElementFromIndex(i);
		_themeCombo->addItem(QString::fromWCharArray(themeInfo.first.c_str()));
	}

	_currentThemeIndex = 0;
	_themeCombo->setCurrentIndex(_currentThemeIndex);
}

void WordStyleDlg::loadFontSizes()
{
	_fontSizeCombo->clear();
	const char* fontSizeStrs[] = {
		"5", "6", "7", "8", "9", "10", "11", "12", "14", "16",
		"18", "20", "22", "24", "26", "28", "36", "48", "72"
	};
	for (const auto* size : fontSizeStrs)
	{
		_fontSizeCombo->addItem(QString::fromUtf8(size));
	}
}

void WordStyleDlg::setStyleListFromLexer(int lexerIndex)
{
	_currentLexerIndex = lexerIndex;
	_styleList->clear();

	if (lexerIndex == 0)
	{
		for (const auto& style : _globalStyles)
		{
			_styleList->addItem(QString::fromWCharArray(style._styleDesc.c_str()));
		}
	}
	else
	{
		LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(static_cast<size_t>(lexerIndex) - 1);
		for (const auto& style : lexerStyler)
		{
			_styleList->addItem(QString::fromWCharArray(style._styleDesc.c_str()));
		}
	}

	if (_styleList->count() > 0)
	{
		_styleList->setCurrentRow(0);
		setVisualFromStyleList();
	}
}

void WordStyleDlg::setVisualFromStyleList()
{
	int styleIndex = _styleList->currentRow();
	if (styleIndex < 0)
		return;

	Style& style = getCurrentStyler();

	_fontCombo->blockSignals(true);
	_fontSizeCombo->blockSignals(true);
	_boldCheck->blockSignals(true);
	_italicCheck->blockSignals(true);
	_underlineCheck->blockSignals(true);

	if (style._isFontEnabled && !style._fontName.empty())
	{
		_fontCombo->setCurrentFont(QFont(QString::fromWCharArray(style._fontName.c_str())));
		_fontCombo->setEnabled(true);
	}
	else
	{
		_fontCombo->setCurrentIndex(-1);
		_fontCombo->setEnabled(true);
	}

	if (style._fontSize != STYLE_NOT_USED && style._fontSize > 0)
	{
		_fontSizeCombo->setCurrentText(QString::number(style._fontSize));
	}
	else
	{
		_fontSizeCombo->setCurrentIndex(-1);
	}

	if (style._fontStyle != STYLE_NOT_USED)
	{
		_boldCheck->setChecked((style._fontStyle & FONTSTYLE_BOLD) != 0);
		_italicCheck->setChecked((style._fontStyle & FONTSTYLE_ITALIC) != 0);
		_underlineCheck->setChecked((style._fontStyle & FONTSTYLE_UNDERLINE) != 0);
	}
	else
	{
		_boldCheck->setChecked(false);
		_italicCheck->setChecked(false);
		_underlineCheck->setChecked(false);
	}

	if (style._fgColor != static_cast<COLORREF>(STYLE_NOT_USED))
	{
		QColor fgColor = colorFromColorRef(style._fgColor);
		_fgColorBtn->setStyleSheet(QString("background-color: %1").arg(fgColor.name()));
	}
	else
	{
		_fgColorBtn->setStyleSheet(QString());
	}

	if (style._bgColor != static_cast<COLORREF>(STYLE_NOT_USED))
	{
		QColor bgColor = colorFromColorRef(style._bgColor);
		_bgColorBtn->setStyleSheet(QString("background-color: %1").arg(bgColor.name()));
	}
	else
	{
		_bgColorBtn->setStyleSheet(QString());
	}

	_userKeywordsEdit->blockSignals(true);
	_userExtEdit->blockSignals(true);

	if (_currentLexerIndex > 0)
	{
		LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(static_cast<size_t>(_currentLexerIndex) - 1);
		_userExtEdit->setText(QString::fromWCharArray(lexerStyler.getLexerUserExt()));
	}
	else
	{
		_userExtEdit->clear();
	}
	_userKeywordsEdit->setText(QString::fromWCharArray(style._keywords.c_str()));

	_userKeywordsEdit->blockSignals(false);
	_userExtEdit->blockSignals(false);
	_fontCombo->blockSignals(false);
	_fontSizeCombo->blockSignals(false);
	_boldCheck->blockSignals(false);
	_italicCheck->blockSignals(false);
	_underlineCheck->blockSignals(false);

	updatePreview();
}

void WordStyleDlg::doDialog()
{
	goToCenter();
	display(true);
}

void WordStyleDlg::prepare2Cancel()
{
	if (_isDirty)
	{
		if (_restoreInvalid)
		{
			NppParameters::getInstance().reloadStylers(_themeName.toStdWString().c_str());
		}

		_lsArray = _styles2restored;
		_globalStyles = _gstyles2restored;
		restoreGlobalOverrideValues();

		_restoreInvalid = false;
		_isDirty = false;
		_isThemeDirty = false;
		_isThemeChanged = false;
	}
}

bool WordStyleDlg::selectThemeByName(const QString& themeName)
{
	for (int i = 0; i < _themeCombo->count(); ++i)
	{
		if (_themeCombo->itemText(i) == themeName)
		{
			_themeCombo->setCurrentIndex(i);
			return true;
		}
	}
	return false;
}

bool WordStyleDlg::goToSection(const QString& section)
{
	int colonPos = section.indexOf(':');
	if (colonPos == -1) return false;

	QString language = section.left(colonPos);
	QString styleName = section.mid(colonPos + 1);

	for (int i = 0; i < _languageCombo->count(); ++i)
	{
		if (_languageCombo->itemText(i) == language)
		{
			_languageCombo->setCurrentIndex(i);
			for (int j = 0; j < _styleList->count(); ++j)
			{
				if (_styleList->item(j)->text() == styleName)
				{
					_styleList->setCurrentRow(j);
					return true;
				}
			}
			break;
		}
	}
	return false;
}

void WordStyleDlg::restoreGlobalOverrideValues()
{
	NppParameters::getInstance().getGlobalOverrideStyle() = _gOverride2restored;
}

void WordStyleDlg::onLanguageChanged(int index)
{
	if (index >= 0)
	{
		bool prevThemeState = _isThemeDirty;
		setStyleListFromLexer(index);
		_isThemeDirty = prevThemeState;
	}
}

void WordStyleDlg::onStyleChanged(int index)
{
	if (index >= 0)
	{
		setVisualFromStyleList();
	}
}

void WordStyleDlg::onFontChanged(const QString& font)
{
	(void)font;
	updateFontName();
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onFontSizeChanged(const QString& size)
{
	(void)size;
	updateFontSize();
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onBoldToggled(bool checked)
{
	(void)checked;
	updateFontStyleStatus(BOLD_STATUS);
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onItalicToggled(bool checked)
{
	(void)checked;
	updateFontStyleStatus(ITALIC_STATUS);
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onUnderlineToggled(bool checked)
{
	(void)checked;
	updateFontStyleStatus(UNDERLINE_STATUS);
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onFgColorClicked()
{
	Style& style = getCurrentStyler();
	QColor initial = Qt::black;
	if (style._fgColor != static_cast<COLORREF>(STYLE_NOT_USED))
		initial = colorFromColorRef(style._fgColor);

	QColor color = QColorDialog::getColor(initial, this, tr("Select Foreground Color"));
	if (color.isValid())
	{
		_fgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
		updateColour(C_FOREGROUND);
		notifyDataModified();
		applyStyleChanges();
	}
}

void WordStyleDlg::onBgColorClicked()
{
	Style& style = getCurrentStyler();
	QColor initial = Qt::white;
	if (style._bgColor != static_cast<COLORREF>(STYLE_NOT_USED))
		initial = colorFromColorRef(style._bgColor);

	QColor color = QColorDialog::getColor(initial, this, tr("Select Background Color"));
	if (color.isValid())
	{
		_bgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
		updateColour(C_BACKGROUND);
		notifyDataModified();
		applyStyleChanges();
	}
}

void WordStyleDlg::onThemeChanged(int index)
{
	if (index >= 0 && index != _currentThemeIndex)
	{
		applyCurrentSelectedThemeAndUpdateUI();
	}
}

void WordStyleDlg::onSaveThemeClicked()
{
	if (_isDirty)
	{
		const LexerStylerArray& lsa = NppParameters::getInstance().getLStylerArray();
		const StyleArray& globalStyles = NppParameters::getInstance().getGlobalStylers();

		_lsArray = lsa;
		_globalStyles = globalStyles;
		updateThemeName(_themeName);
		_restoreInvalid = false;

		_currentThemeIndex = _themeCombo->currentIndex();
		_isDirty = false;
		_isThemeChanged = false;
	}
	_isThemeDirty = false;

	auto newSavedFilePath = NppParameters::getInstance().writeStyles(_lsArray, _globalStyles);
	if (!newSavedFilePath.empty())
		updateThemeName(QString::fromWCharArray(newSavedFilePath.c_str()));

	_saveThemeBtn->setEnabled(false);
	display(false);
}

void WordStyleDlg::onSaveAsThemeClicked()
{
	ThemeSwitcher& themeSwitcher = NppParameters::getInstance().getThemeSwitcher();
	QString defaultDir = QString::fromWCharArray(themeSwitcher.getThemeDirPath().c_str());
	if (defaultDir.isEmpty())
	{
		defaultDir = _themeName;
		int lastSlash = defaultDir.lastIndexOf('/');
		if (lastSlash >= 0)
			defaultDir = defaultDir.left(lastSlash);
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Theme As"),
													defaultDir, tr("XML files (*.xml)"));
	if (!fileName.isEmpty())
	{
		NppParameters& nppParams = NppParameters::getInstance();
		nppParams.writeStyles(_lsArray, _globalStyles);

		std::wstring srcPath = nppParams.getNppGUI()._themeName;
		if (!srcPath.empty())
		{
			QFile::remove(fileName);
			QFile::copy(QString::fromWCharArray(srcPath.c_str()), fileName);
		}

		updateThemeName(fileName);

		std::wstring newThemePath = fileName.toStdWString();
		std::wstring newThemeName = themeSwitcher.getThemeFromXmlFileName(newThemePath.c_str());
		if (!newThemeName.empty() && !themeSwitcher.themeNameExists(newThemeName.c_str()))
		{
			themeSwitcher.addThemeFromXml(newThemePath);
			_themeCombo->addItem(QString::fromWCharArray(newThemeName.c_str()));
		}

		_isDirty = false;
		_isThemeDirty = false;
		_saveThemeBtn->setEnabled(false);
	}
}

void WordStyleDlg::onGlobalFgToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableFg = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalBgToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableBg = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalFontToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableFont = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalFontSizeToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableFontSize = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalBoldToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableBold = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalItalicToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableItalic = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGlobalUnderlineToggled(bool checked)
{
	NppParameters::getInstance().getGlobalOverrideStyle().enableUnderLine = checked;
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onGoToSettingsClicked()
{
	// Navigation to preferences - parent handles this
}

void WordStyleDlg::onOkClicked()
{
	onSaveThemeClicked();
}

void WordStyleDlg::onCancelClicked()
{
	prepare2Cancel();
	display(false);
}

void WordStyleDlg::onUserKeywordsChanged()
{
	updateUserKeywords();
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::onUserExtChanged()
{
	updateExtension();
	notifyDataModified();
	applyStyleChanges();
}

void WordStyleDlg::updateFontName()
{
	Style& style = getCurrentStyler();
	style._fontName = _fontCombo->currentFont().family().toStdWString();
	style._isFontEnabled = true;
}

void WordStyleDlg::updateFontSize()
{
	Style& style = getCurrentStyler();
	bool ok = false;
	int size = _fontSizeCombo->currentText().toInt(&ok);
	if (ok && size > 0)
		style._fontSize = size;
}

void WordStyleDlg::updateFontStyleStatus(fontStyleType styleType)
{
	Style& style = getCurrentStyler();
	int fontStyle = style._fontStyle;

	if (fontStyle == STYLE_NOT_USED)
		fontStyle = 0;

	switch (styleType)
	{
		case BOLD_STATUS:
			if (_boldCheck->isChecked())
				fontStyle |= FONTSTYLE_BOLD;
			else
				fontStyle &= ~FONTSTYLE_BOLD;
			break;
		case ITALIC_STATUS:
			if (_italicCheck->isChecked())
				fontStyle |= FONTSTYLE_ITALIC;
			else
				fontStyle &= ~FONTSTYLE_ITALIC;
			break;
		case UNDERLINE_STATUS:
			if (_underlineCheck->isChecked())
				fontStyle |= FONTSTYLE_UNDERLINE;
			else
				fontStyle &= ~FONTSTYLE_UNDERLINE;
			break;
	}
	style._fontStyle = fontStyle;
}

void WordStyleDlg::updateColour(bool isBackground)
{
	Style& style = getCurrentStyler();
	if (isBackground)
	{
		QString ss = _bgColorBtn->styleSheet();
		int idx = ss.indexOf('#');
		if (idx >= 0)
		{
			QColor c(ss.mid(idx, 7));
			if (c.isValid())
				style._bgColor = colorRefFromColor(c);
		}
	}
	else
	{
		QString ss = _fgColorBtn->styleSheet();
		int idx = ss.indexOf('#');
		if (idx >= 0)
		{
			QColor c(ss.mid(idx, 7));
			if (c.isValid())
				style._fgColor = colorRefFromColor(c);
		}
	}
}

void WordStyleDlg::updateUserKeywords()
{
	Style& style = getCurrentStyler();
	style._keywords = _userKeywordsEdit->text().toStdWString();
}

void WordStyleDlg::updateExtension()
{
	if (_currentLexerIndex > 0)
	{
		LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(static_cast<size_t>(_currentLexerIndex) - 1);
		lexerStyler.setLexerUserExt(_userExtEdit->text().toStdWString().c_str());
	}
}

void WordStyleDlg::updateGlobalOverrideCtrls()
{
	const GlobalOverride& glo = NppParameters::getInstance().getGlobalOverrideStyle();

	_globalFgCheck->blockSignals(true);
	_globalBgCheck->blockSignals(true);
	_globalFontCheck->blockSignals(true);
	_globalFontSizeCheck->blockSignals(true);
	_globalBoldCheck->blockSignals(true);
	_globalItalicCheck->blockSignals(true);
	_globalUnderlineCheck->blockSignals(true);

	_globalFgCheck->setChecked(glo.enableFg);
	_globalBgCheck->setChecked(glo.enableBg);
	_globalFontCheck->setChecked(glo.enableFont);
	_globalFontSizeCheck->setChecked(glo.enableFontSize);
	_globalBoldCheck->setChecked(glo.enableBold);
	_globalItalicCheck->setChecked(glo.enableItalic);
	_globalUnderlineCheck->setChecked(glo.enableUnderLine);

	_globalFgCheck->blockSignals(false);
	_globalBgCheck->blockSignals(false);
	_globalFontCheck->blockSignals(false);
	_globalFontSizeCheck->blockSignals(false);
	_globalBoldCheck->blockSignals(false);
	_globalItalicCheck->blockSignals(false);
	_globalUnderlineCheck->blockSignals(false);
}

void WordStyleDlg::applyStyleChanges()
{
	updatePreview();
}

void WordStyleDlg::updatePreview()
{
	if (_preview)
	{
		int styleIndex = _styleList->currentRow();
		if (styleIndex >= 0)
		{
			Style& style = getCurrentStyler();
			_preview->applyStyle(style);
		}
	}
}

void WordStyleDlg::notifyDataModified()
{
	_isDirty = true;
	_isThemeDirty = true;
	_saveThemeBtn->setEnabled(true);
}

void WordStyleDlg::switchToTheme(int themeIndex)
{
	NppParameters& nppParams = NppParameters::getInstance();

	if (_isThemeDirty)
	{
		int response = QMessageBox::question(this,
			tr("Unsaved Changes"),
			tr("Unsaved changes are about to be discarded!\n"
			   "Do you want to save your changes before switching themes?"),
			QMessageBox::Yes | QMessageBox::No);
		if (response == QMessageBox::Yes)
		{
			nppParams.writeStyles(_lsArray, _globalStyles);
		}
	}

	ThemeSwitcher& themeSwitcher = nppParams.getThemeSwitcher();
	if (static_cast<size_t>(themeIndex) < themeSwitcher.size())
	{
		auto& themeInfo = themeSwitcher.getElementFromIndex(static_cast<size_t>(themeIndex));
		_themeName = QString::fromWCharArray(themeInfo.second.c_str());
		nppParams.reloadStylers(themeInfo.second.c_str());
	}

	_lsArray = nppParams.getLStylerArray();
	_globalStyles = nppParams.getGlobalStylers();

	loadLanguages();
	setVisualFromStyleList();

	_currentThemeIndex = themeIndex;
	_isThemeChanged = true;
	_isThemeDirty = false;
	_restoreInvalid = true;
}

void WordStyleDlg::updateThemeName(const QString& themeName)
{
	_themeName = themeName;
	NppParameters::getInstance().getNppGUI()._themeName = themeName.toStdWString();
}

void WordStyleDlg::applyCurrentSelectedThemeAndUpdateUI()
{
	int newThemeIndex = _themeCombo->currentIndex();
	if (newThemeIndex != _currentThemeIndex)
	{
		switchToTheme(newThemeIndex);
		notifyDataModified();
	}
}

Style& WordStyleDlg::getCurrentStyler()
{
	int styleIndex = _styleList->currentRow();
	if (styleIndex < 0)
	{
		static Style dummyStyle;
		return dummyStyle;
	}

	if (_currentLexerIndex == 0)
	{
		return _globalStyles.getStyler(static_cast<size_t>(styleIndex));
	}
	else
	{
		LexerStyler& lexerStyler = _lsArray.getLexerFromIndex(static_cast<size_t>(_currentLexerIndex) - 1);
		return lexerStyler.getStyler(static_cast<size_t>(styleIndex));
	}
}

const Style& WordStyleDlg::getCurrentStyler() const
{
	return const_cast<WordStyleDlg*>(this)->getCurrentStyler();
}

bool WordStyleDlg::getCurrentStyleName(QString& styleName) const
{
	int row = _styleList->currentRow();
	if (row < 0) return false;
	styleName = _styleList->item(row)->text();
	return true;
}

int WordStyleDlg::getApplicationInfo() const
{
	QString styleName;
	if (!getCurrentStyleName(styleName))
	{
		return 0;
	}

	if (styleName == tr("Default Style"))
	{
		return 1 | 2;
	}

	if (styleName.contains(tr("Mark Style")) ||
		styleName.contains(tr("Tab colour")))
	{
		return 1 | 4;
	}

	return 1;
}

int WordStyleDlg::whichTabColourIndex() const
{
	QString styleName;
	if (!getCurrentStyleName(styleName)) return -1;

	if (styleName == tr("Active tab text colour")) return 0;
	if (styleName == tr("Active tab background colour")) return 1;
	if (styleName == tr("Inactive tabs")) return 2;

	return -1;
}

int WordStyleDlg::whichIndividualTabColourId() const
{
	QString styleName;
	if (!getCurrentStyleName(styleName)) return -1;

	for (int i = 1; i <= 5; ++i)
	{
		if (styleName == tr("Tab colour %1").arg(i))
		{
			return i - 1;
		}
	}

	return -1;
}

int WordStyleDlg::whichFindDlgStatusMsgColourIndex() const
{
	QString styleName;
	if (!getCurrentStyleName(styleName)) return -1;

	if (styleName.contains(tr("Find status")))
	{
		if (styleName.contains(tr("Not found"))) return 0;
		if (styleName.contains(tr("Message"))) return 1;
		if (styleName.contains(tr("Search end reached"))) return 2;
	}

	return -1;
}

bool WordStyleDlg::isDocumentMapStyle() const
{
	QString styleName;
	if (!getCurrentStyleName(styleName)) return false;
	return (styleName == tr("Document Map"));
}

void WordStyleDlg::enableFontControls(bool enable)
{
	_fontCombo->setEnabled(enable);
	_fontSizeCombo->setEnabled(enable);
	_boldCheck->setEnabled(enable);
	_italicCheck->setEnabled(enable);
	_underlineCheck->setEnabled(enable);
}

void WordStyleDlg::enableColorControls(bool enable)
{
	_fgColorBtn->setEnabled(enable);
	_bgColorBtn->setEnabled(enable);
}

QColor WordStyleDlg::colorFromColorRef(COLORREF color) const
{
	return QColor(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF WordStyleDlg::colorRefFromColor(const QColor& color) const
{
	return RGB(color.red(), color.green(), color.blue());
}

// ============================================================================
// ScintillaPreview implementation
// ============================================================================

ScintillaPreview::ScintillaPreview(QWidget* parent)
	: QWidget(parent)
{
	setMinimumSize(200, 100);
}

ScintillaPreview::~ScintillaPreview() = default;

void ScintillaPreview::setSampleText(const QString& text)
{
	_sampleText = text;
	update();
}

void ScintillaPreview::setLexer(int lexerId)
{
	_lexerId = lexerId;
	update();
}

void ScintillaPreview::applyStyle(const Style& style)
{
	(void)style;
	update();
}

void ScintillaPreview::clearStyles()
{
	_styles.clear();
	update();
}

void ScintillaPreview::paintEvent(QPaintEvent* event)
{
	(void)event;
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);

	QFont font("Consolas", 10);
	painter.setFont(font);
	painter.setPen(Qt::black);

	QString previewText = _sampleText.isEmpty()
		? tr("// Sample preview text\nfunction example() {\n    return 42;\n}")
		: _sampleText;

	int y = 20;
	for (const QString& line : previewText.split('\n'))
	{
		painter.drawText(10, y, line);
		y += painter.fontMetrics().height();
	}
}

} // namespace QtControls
