// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ColumnEditorDlg.h"
#include "NppConstants.h"
#include "ScintillaEditView.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtGui/QKeyEvent>
#include <QtCore/QTimer>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QToolTip>

namespace QtControls {

ColumnEditorDlg::ColumnEditorDlg(QWidget* parent)
    : StaticDialog(parent)
{
}

ColumnEditorDlg::~ColumnEditorDlg() = default;

void ColumnEditorDlg::init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView)
{
    Q_UNUSED(hInst);
    Q_UNUSED(hPere);

    // Store the edit view pointer for later use
    if (!ppEditView)
        throw std::runtime_error("ColumnEditorDlg::init : ppEditView is null.");
    _ppEditView = ppEditView;
}

void ColumnEditorDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Column Editor"));
    dialog->resize(400, 450);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // === Text to Insert Section ===
    _textRadio = new QRadioButton(tr("Text to Insert"), dialog);
    mainLayout->addWidget(_textRadio);

    _textGroup = new QGroupBox(dialog);
    auto* textLayout = new QVBoxLayout(_textGroup);
    textLayout->setContentsMargins(8, 8, 8, 8);

    _textEdit = new QLineEdit(dialog);
    _textEdit->setPlaceholderText(tr("Enter text to insert"));
    textLayout->addWidget(_textEdit);

    mainLayout->addWidget(_textGroup);

    // === Number to Insert Section ===
    _numRadio = new QRadioButton(tr("Number to Insert"), dialog);
    mainLayout->addWidget(_numRadio);

    _numGroup = new QGroupBox(dialog);
    auto* numLayout = new QGridLayout(_numGroup);
    numLayout->setContentsMargins(8, 8, 8, 8);
    numLayout->setSpacing(6);

    // Initial number
    _initNumLabel = new QLabel(tr("Initial number:"), dialog);
    numLayout->addWidget(_initNumLabel, 0, 0);
    _initNumEdit = new QLineEdit(dialog);
    _initNumEdit->setText("0");
    numLayout->addWidget(_initNumEdit, 0, 1);

    // Increase by
    _incrNumLabel = new QLabel(tr("Increase by:"), dialog);
    numLayout->addWidget(_incrNumLabel, 1, 0);
    _incrNumEdit = new QLineEdit(dialog);
    _incrNumEdit->setText("1");
    numLayout->addWidget(_incrNumEdit, 1, 1);

    // Repeat
    _repeatNumLabel = new QLabel(tr("Repeat:"), dialog);
    numLayout->addWidget(_repeatNumLabel, 2, 0);
    _repeatNumEdit = new QLineEdit(dialog);
    _repeatNumEdit->setText("1");
    numLayout->addWidget(_repeatNumEdit, 2, 1);

    mainLayout->addWidget(_numGroup);

    // === Format Section ===
    _formatGroup = new QGroupBox(tr("Format"), dialog);
    auto* formatLayout = new QHBoxLayout(_formatGroup);
    formatLayout->setSpacing(10);

    _decRadio = new QRadioButton(tr("Dec"), dialog);
    _decRadio->setChecked(true);
    formatLayout->addWidget(_decRadio);

    _hexRadio = new QRadioButton(tr("Hex"), dialog);
    formatLayout->addWidget(_hexRadio);

    _octRadio = new QRadioButton(tr("Oct"), dialog);
    formatLayout->addWidget(_octRadio);

    _binRadio = new QRadioButton(tr("Bin"), dialog);
    formatLayout->addWidget(_binRadio);

    formatLayout->addStretch();

    mainLayout->addWidget(_formatGroup);

    // === Leading and Hex Case Section ===
    auto* optionsLayout = new QHBoxLayout();

    // Leading combo
    _leadingLabel = new QLabel(tr("Leading:"), dialog);
    optionsLayout->addWidget(_leadingLabel);

    _leadingCombo = new QComboBox(dialog);
    _leadingCombo->addItem(tr("None"));
    _leadingCombo->addItem(tr("Zeros"));
    _leadingCombo->addItem(tr("Spaces"));
    optionsLayout->addWidget(_leadingCombo);

    optionsLayout->addSpacing(20);

    // Hex case combo
    _hexCaseCombo = new QComboBox(dialog);
    _hexCaseCombo->addItem("a-f");
    _hexCaseCombo->addItem("A-F");
    _hexCaseCombo->setEnabled(false);
    optionsLayout->addWidget(_hexCaseCombo);

    optionsLayout->addStretch();

    mainLayout->addLayout(optionsLayout);

    // Add stretch to push buttons to bottom
    mainLayout->addStretch();

    // === Button Section ===
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _okButton = new QPushButton(tr("OK"), dialog);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();

    // Initialize from parameters
    ColumnEditorParam colEditParam = NppParameters::getInstance()._columnEditParam;

    // Set text content
    _textEdit->setText(QString::fromStdWString(colEditParam._insertedTextContent));

    // Set numeric fields
    setNumericFields(colEditParam);

    // Set leading choice
    switch (colEditParam._leadingChoice)
    {
        case ColumnEditorParam::noneLeading:
        default:
            _leadingCombo->setCurrentIndex(0);
            break;
        case ColumnEditorParam::zeroLeading:
            _leadingCombo->setCurrentIndex(1);
            break;
        case ColumnEditorParam::spaceLeading:
            _leadingCombo->setCurrentIndex(2);
            break;
    }

    // Set format radio buttons
    int format = colEditParam._formatChoice;
    if (format == BASE_10)
        _decRadio->setChecked(true);
    else if (format == BASE_16 || format == BASE_16_UPPERCASE)
        _hexRadio->setChecked(true);
    else if (format == BASE_08)
        _octRadio->setChecked(true);
    else if (format == BASE_02)
        _binRadio->setChecked(true);

    // Set hex case
    _hexCaseCombo->setCurrentIndex((format == BASE_16_UPPERCASE) ? 1 : 0);

    // Switch to correct mode
    switchTo(colEditParam._mainChoice);
}

void ColumnEditorDlg::connectSignals()
{
    connect(_okButton, &QPushButton::clicked, this, &ColumnEditorDlg::onOKClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &ColumnEditorDlg::onCancelClicked);

    connect(_textRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onModeChanged);
    connect(_numRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onModeChanged);

    connect(_decRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onFormatChanged);
    connect(_hexRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onFormatChanged);
    connect(_octRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onFormatChanged);
    connect(_binRadio, &QRadioButton::toggled, this, &ColumnEditorDlg::onFormatChanged);

    connect(_textEdit, &QLineEdit::textChanged, this, &ColumnEditorDlg::onTextChanged);

    connect(_leadingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ColumnEditorDlg::onLeadingChanged);

    connect(_hexCaseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ColumnEditorDlg::onHexCaseChanged);

    // Connect numeric field changes for validation
    connect(_initNumEdit, &QLineEdit::editingFinished, this, &ColumnEditorDlg::onNumericFieldChanged);
    connect(_incrNumEdit, &QLineEdit::editingFinished, this, &ColumnEditorDlg::onNumericFieldChanged);
    connect(_repeatNumEdit, &QLineEdit::editingFinished, this, &ColumnEditorDlg::onNumericFieldChanged);
}

void ColumnEditorDlg::doDialog(bool isRTL)
{
    Q_UNUSED(isRTL);

    if (!isCreated()) {
        create(tr("Column Editor"), false);
        setupUI();
        connectSignals();
    }

    const bool isTextMode = _textRadio->isChecked();
    display(true);

    // Set focus to appropriate field
    if (isTextMode) {
        _textEdit->setFocus();
        _textEdit->selectAll();
    } else {
        _initNumEdit->setFocus();
        _initNumEdit->selectAll();
    }
}

void ColumnEditorDlg::display(bool toShow, bool enhancedPositioningCheckWhenShowing)
{
    StaticDialog::display(toShow, enhancedPositioningCheckWhenShowing);
}

void ColumnEditorDlg::switchTo(bool toText)
{
    // Enable/disable text controls
    _textEdit->setEnabled(toText);
    _textRadio->setChecked(toText);

    // Enable/disable number controls
    _numRadio->setChecked(!toText);
    _initNumEdit->setEnabled(!toText);
    _incrNumEdit->setEnabled(!toText);
    _repeatNumEdit->setEnabled(!toText);
    _decRadio->setEnabled(!toText);
    _hexRadio->setEnabled(!toText);
    _octRadio->setEnabled(!toText);
    _binRadio->setEnabled(!toText);
    _leadingCombo->setEnabled(!toText);
    _hexCaseCombo->setEnabled(!toText && _hexRadio->isChecked());

    // Update label enabled states
    _initNumLabel->setEnabled(!toText);
    _incrNumLabel->setEnabled(!toText);
    _repeatNumLabel->setEnabled(!toText);
    _leadingLabel->setEnabled(!toText);

    // Update OK button state
    if (toText) {
        _okButton->setEnabled(!_textEdit->text().isEmpty());
    } else {
        _okButton->setEnabled(true);
    }

    // Set focus
    if (toText) {
        _textEdit->setFocus();
    } else {
        _initNumEdit->setFocus();
    }
}

UCHAR ColumnEditorDlg::getFormat()
{
    UCHAR f = BASE_10; // Dec by default
    if (_hexRadio->isChecked())
        f = getHexCase(); // Will give BASE_16 or BASE_16_UC
    else if (_octRadio->isChecked())
        f = BASE_08;
    else if (_binRadio->isChecked())
        f = BASE_02;
    return f;
}

UCHAR ColumnEditorDlg::getHexCase()
{
    int curSel = _hexCaseCombo->currentIndex();
    return (curSel == 1) ? BASE_16_UPPERCASE : BASE_16;
}

ColumnEditorParam::leadingChoice ColumnEditorDlg::getLeading()
{
    ColumnEditorParam::leadingChoice leading = ColumnEditorParam::noneLeading;
    int curSel = _leadingCombo->currentIndex();
    switch (curSel)
    {
        case 0:
        default:
            leading = ColumnEditorParam::noneLeading;
            break;
        case 1:
            leading = ColumnEditorParam::zeroLeading;
            break;
        case 2:
            leading = ColumnEditorParam::spaceLeading;
            break;
    }
    return leading;
}

void ColumnEditorDlg::setNumericFields(const ColumnEditorParam& colEditParam)
{
    if (colEditParam._formatChoice == BASE_10)
    {
        if (colEditParam._initialNum != -1)
            _initNumEdit->setText(QString::number(colEditParam._initialNum));
        else
            _initNumEdit->clear();

        if (colEditParam._increaseNum != -1)
            _incrNumEdit->setText(QString::number(colEditParam._increaseNum));
        else
            _incrNumEdit->clear();

        if (colEditParam._repeatNum != -1)
            _repeatNumEdit->setText(QString::number(colEditParam._repeatNum));
        else
            _repeatNumEdit->clear();
    }
    else
    {
        size_t base = 10;
        switch (colEditParam._formatChoice)
        {
            case BASE_16:
            case BASE_16_UPPERCASE:
                base = 16;
                break;
            case BASE_08:
                base = 8;
                break;
            case BASE_02:
                base = 2;
                break;
            default:
                base = 10;
                break;
        }
        bool useUpper = (colEditParam._formatChoice == BASE_16_UPPERCASE);

        if (colEditParam._initialNum != -1)
        {
            QString str = QString::number(colEditParam._initialNum, static_cast<int>(base));
            if (useUpper)
                str = str.toUpper();
            _initNumEdit->setText(str);
        }
        else
            _initNumEdit->clear();

        if (colEditParam._increaseNum != -1)
        {
            QString str = QString::number(colEditParam._increaseNum, static_cast<int>(base));
            if (useUpper)
                str = str.toUpper();
            _incrNumEdit->setText(str);
        }
        else
            _incrNumEdit->clear();

        if (colEditParam._repeatNum != -1)
        {
            QString str = QString::number(colEditParam._repeatNum, static_cast<int>(base));
            if (useUpper)
                str = str.toUpper();
            _repeatNumEdit->setText(str);
        }
        else
            _repeatNumEdit->clear();
    }
}

int ColumnEditorDlg::getNumericFieldValueFromText(int formatChoice, const QString& str)
{
    if (str.isEmpty())
        return 0;

    int base = 10;
    switch (formatChoice)
    {
        case BASE_16:
        case BASE_16_UPPERCASE:
            base = 16;
            break;
        case BASE_08:
            base = 8;
            break;
        case BASE_02:
            base = 2;
            break;
        default:
            base = 10;
            break;
    }

    bool ok = false;
    int num = str.toInt(&ok, base);
    if (!ok)
        return -1;

    return num;
}

void ColumnEditorDlg::showValidationError(int whichField, int formatChoice, const QString& str)
{
    QString radixNote;
    switch (formatChoice)
    {
        case BASE_16:
        case BASE_16_UPPERCASE:
            radixNote = tr("Hex numbers use 0-9, A-F!");
            break;
        case BASE_08:
            radixNote = tr("Oct numbers only use 0-7!");
            break;
        case BASE_02:
            radixNote = tr("Bin numbers only use 0-1!");
            break;
        default:
            radixNote = tr("Decimal numbers only use 0-9!");
            break;
    }

    QString msg;
    if (!str.isEmpty())
        msg = tr("Entered string \"%1\":\n%2").arg(str).arg(radixNote);
    else
        msg = radixNote;

    // Show tooltip at the appropriate field
    QLineEdit* field = nullptr;
    switch (whichField)
    {
        case FIELD_INIT_NUM:
            field = _initNumEdit;
            break;
        case FIELD_INCREASE_NUM:
            field = _incrNumEdit;
            break;
        case FIELD_REPEAT_NUM:
            field = _repeatNumEdit;
            break;
    }

    if (field) {
        QToolTip::showText(field->mapToGlobal(QPoint(0, field->height())), msg, field);

        // Flash the field background red
        _flashFieldId = whichField;
        field->setStyleSheet("QLineEdit { background-color: #FF0000; color: white; }");

        QTimer::singleShot(250, this, [this, field]() {
            field->setStyleSheet("");
            _flashFieldId = FIELD_NONE;
        });

        QTimer::singleShot(3500, this, [field]() {
            QToolTip::hideText();
        });
    }
}

void ColumnEditorDlg::onOKClicked()
{
    if (!_ppEditView || !*_ppEditView)
        return;

    ScintillaEditView* view = *_ppEditView;
    view->execute(SCI_BEGINUNDOACTION);

    bool isTextMode = _textRadio->isChecked();

    if (isTextMode)
    {
        QString text = _textEdit->text();
        std::wstring str = text.toStdWString();

        display(false);

        if (view->execute(SCI_SELECTIONISRECTANGLE) || view->execute(SCI_GETSELECTIONS) > 1)
        {
            ColumnModeInfos colInfos = view->getColumnModeSelectInfo();
            std::sort(colInfos.begin(), colInfos.end(), SortInPositionOrder());
            view->columnReplace(colInfos, str.c_str());
            std::sort(colInfos.begin(), colInfos.end(), SortInSelectOrder());
            view->setMultiSelections(colInfos);
        }
        else
        {
            auto cursorPos = view->execute(SCI_GETCURRENTPOS);
            auto cursorCol = view->execute(SCI_GETCOLUMN, cursorPos);
            auto cursorLine = view->execute(SCI_LINEFROMPOSITION, cursorPos);
            auto endPos = view->execute(SCI_GETLENGTH);
            auto endLine = view->execute(SCI_LINEFROMPOSITION, endPos);

            constexpr int lineAllocatedLen = 1024;
            wchar_t* line = new wchar_t[lineAllocatedLen];

            for (size_t i = cursorLine; i <= static_cast<size_t>(endLine); ++i)
            {
                auto lineBegin = view->execute(SCI_POSITIONFROMLINE, i);
                auto lineEnd = view->execute(SCI_GETLINEENDPOSITION, i);

                auto lineEndCol = view->execute(SCI_GETCOLUMN, lineEnd);
                auto lineLen = lineEnd - lineBegin + 1;

                if (lineLen > lineAllocatedLen)
                {
                    delete[] line;
                    line = new wchar_t[lineLen];
                }
                view->getGenericText(line, lineLen, lineBegin, lineEnd);
                std::wstring s2r(line);

                if (lineEndCol < static_cast<long long>(cursorCol))
                {
                    std::wstring s_space(cursorCol - lineEndCol, ' ');
                    s2r.append(s_space);
                    s2r.append(str);
                }
                else
                {
                    auto posAbs2Start = view->execute(SCI_FINDCOLUMN, i, cursorCol);
                    auto posRelative2Start = posAbs2Start - lineBegin;
                    if (posRelative2Start > static_cast<long long>(s2r.length()))
                        posRelative2Start = s2r.length();

                    s2r.insert(posRelative2Start, str);
                }
                view->replaceTarget(s2r.c_str(), static_cast<int>(lineBegin), static_cast<int>(lineEnd));
            }
            delete[] line;
        }
    }
    else
    {
        ColumnEditorParam colEditParam = NppParameters::getInstance()._columnEditParam;

        int initialNumber = getNumericFieldValueFromText(colEditParam._formatChoice, _initNumEdit->text());
        if (initialNumber == -1)
        {
            showValidationError(FIELD_INIT_NUM, colEditParam._formatChoice, _initNumEdit->text());
            view->execute(SCI_ENDUNDOACTION);
            return;
        }

        int increaseNumber = getNumericFieldValueFromText(colEditParam._formatChoice, _incrNumEdit->text());
        if (increaseNumber == -1)
        {
            showValidationError(FIELD_INCREASE_NUM, colEditParam._formatChoice, _incrNumEdit->text());
            view->execute(SCI_ENDUNDOACTION);
            return;
        }

        int repeat = getNumericFieldValueFromText(colEditParam._formatChoice, _repeatNumEdit->text());
        if (repeat == -1)
        {
            showValidationError(FIELD_REPEAT_NUM, colEditParam._formatChoice, _repeatNumEdit->text());
            view->execute(SCI_ENDUNDOACTION);
            return;
        }

        if (repeat == 0)
            repeat = 1;

        UCHAR format = getFormat();
        display(false);

        if (view->execute(SCI_SELECTIONISRECTANGLE) || view->execute(SCI_GETSELECTIONS) > 1)
        {
            ColumnModeInfos colInfos = view->getColumnModeSelectInfo();

            if (colInfos.size() > 0)
            {
                std::sort(colInfos.begin(), colInfos.end(), SortInPositionOrder());
                view->columnReplace(colInfos, initialNumber, increaseNumber, repeat, format, getLeading());
                std::sort(colInfos.begin(), colInfos.end(), SortInSelectOrder());
                view->setMultiSelections(colInfos);
            }
        }
        else
        {
            auto cursorPos = view->execute(SCI_GETCURRENTPOS);
            auto cursorCol = view->execute(SCI_GETCOLUMN, cursorPos);
            auto cursorLine = view->execute(SCI_LINEFROMPOSITION, cursorPos);
            auto endPos = view->execute(SCI_GETLENGTH);
            auto endLine = view->execute(SCI_LINEFROMPOSITION, endPos);

            // Compute the numbers to be placed at each column
            std::vector<size_t> numbers;

            size_t curNumber = initialNumber;
            const size_t kiMaxSize = 1 + static_cast<size_t>(endLine) - static_cast<size_t>(cursorLine);
            while (numbers.size() < kiMaxSize)
            {
                for (int i = 0; i < repeat; i++)
                {
                    numbers.push_back(curNumber);

                    if (numbers.size() >= kiMaxSize)
                        break;
                }
                curNumber += increaseNumber;
            }

            constexpr int lineAllocatedLen = 1024;
            wchar_t* line = new wchar_t[lineAllocatedLen];

            size_t base = 10;
            bool useUppercase = false;
            if (format == BASE_16)
                base = 16;
            else if (format == BASE_08)
                base = 8;
            else if (format == BASE_02)
                base = 2;
            else if (format == BASE_16_UPPERCASE)
            {
                base = 16;
                useUppercase = true;
            }

            size_t endNumber = *numbers.rbegin();
            size_t nbEnd = getNbDigits(endNumber, base);
            size_t nbInit = getNbDigits(initialNumber, base);
            size_t nb = std::max<size_t>(nbInit, nbEnd);

            for (size_t i = cursorLine; i <= static_cast<size_t>(endLine); ++i)
            {
                auto lineBegin = view->execute(SCI_POSITIONFROMLINE, i);
                auto lineEnd = view->execute(SCI_GETLINEENDPOSITION, i);

                auto lineEndCol = view->execute(SCI_GETCOLUMN, lineEnd);
                auto lineLen = lineEnd - lineBegin + 1;

                if (lineLen > lineAllocatedLen)
                {
                    delete[] line;
                    line = new wchar_t[lineLen];
                }
                view->getGenericText(line, lineLen, lineBegin, lineEnd);

                std::wstring s2r(line);

                // Format the number
                constexpr int stringSize = 1024;
                wchar_t str[stringSize]{};
                variedFormatNumber2String<wchar_t>(str, stringSize, numbers.at(i - cursorLine), base, useUppercase, nb, getLeading());

                if (lineEndCol < static_cast<long long>(cursorCol))
                {
                    std::wstring s_space(cursorCol - lineEndCol, ' ');
                    s2r.append(s_space);
                    s2r.append(str);
                }
                else
                {
                    auto posAbs2Start = view->execute(SCI_FINDCOLUMN, i, cursorCol);
                    auto posRelative2Start = posAbs2Start - lineBegin;
                    if (posRelative2Start > static_cast<long long>(s2r.length()))
                        posRelative2Start = s2r.length();

                    s2r.insert(posRelative2Start, str);
                }

                view->replaceTarget(s2r.c_str(), static_cast<int>(lineBegin), static_cast<int>(lineEnd));
            }
            delete[] line;
        }
    }

    view->execute(SCI_ENDUNDOACTION);
    view->grabFocus();
}

void ColumnEditorDlg::onCancelClicked()
{
    display(false);
}

void ColumnEditorDlg::onModeChanged()
{
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;
    colEditParam._mainChoice = _textRadio->isChecked() ? activeText : activeNumeric;
    switchTo(colEditParam._mainChoice);
}

void ColumnEditorDlg::onFormatChanged()
{
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;

    if (_decRadio->isChecked())
        colEditParam._formatChoice = BASE_10;
    else if (_hexRadio->isChecked())
        colEditParam._formatChoice = getHexCase();
    else if (_octRadio->isChecked())
        colEditParam._formatChoice = BASE_08;
    else if (_binRadio->isChecked())
        colEditParam._formatChoice = BASE_02;

    setNumericFields(colEditParam);
    _hexCaseCombo->setEnabled(_hexRadio->isChecked());
}

void ColumnEditorDlg::onTextChanged(const QString& text)
{
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;
    colEditParam._insertedTextContent = text.toStdWString();

    if (_textRadio->isChecked()) {
        _okButton->setEnabled(!text.isEmpty());
    }
}

void ColumnEditorDlg::onLeadingChanged(int index)
{
    Q_UNUSED(index);
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;
    colEditParam._leadingChoice = getLeading();
}

void ColumnEditorDlg::onHexCaseChanged(int index)
{
    Q_UNUSED(index);
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;
    if ((colEditParam._formatChoice & BASE_16) == BASE_16)
        colEditParam._formatChoice = getHexCase();

    setNumericFields(colEditParam);
}

void ColumnEditorDlg::onNumericFieldChanged()
{
    // Validate numeric fields
    ColumnEditorParam& colEditParam = NppParameters::getInstance()._columnEditParam;

    QLineEdit* senderField = qobject_cast<QLineEdit*>(sender());
    if (!senderField) return;

    QString text = senderField->text();
    if (text.isEmpty()) return;

    int num = getNumericFieldValueFromText(colEditParam._formatChoice, text);
    if (num == -1)
    {
        // Invalid input - restore previous value
        setNumericFields(colEditParam);

        int whichField = FIELD_NONE;
        if (senderField == _initNumEdit)
            whichField = FIELD_INIT_NUM;
        else if (senderField == _incrNumEdit)
            whichField = FIELD_INCREASE_NUM;
        else if (senderField == _repeatNumEdit)
            whichField = FIELD_REPEAT_NUM;

        showValidationError(whichField, colEditParam._formatChoice, text);
    }
    else
    {
        // Valid input - store the value
        if (senderField == _initNumEdit)
            colEditParam._initialNum = num;
        else if (senderField == _incrNumEdit)
            colEditParam._increaseNum = num;
        else if (senderField == _repeatNumEdit)
            colEditParam._repeatNum = num;
    }
}

void ColumnEditorDlg::updateEnabledStates()
{
    bool isTextMode = _textRadio->isChecked();

    _textEdit->setEnabled(isTextMode);
    _initNumEdit->setEnabled(!isTextMode);
    _incrNumEdit->setEnabled(!isTextMode);
    _repeatNumEdit->setEnabled(!isTextMode);
    _decRadio->setEnabled(!isTextMode);
    _hexRadio->setEnabled(!isTextMode);
    _octRadio->setEnabled(!isTextMode);
    _binRadio->setEnabled(!isTextMode);
    _leadingCombo->setEnabled(!isTextMode);
    _hexCaseCombo->setEnabled(!isTextMode && _hexRadio->isChecked());
}

bool ColumnEditorDlg::run_dlgProc(QEvent* event)
{
    Q_UNUSED(event);
    return true;
}

} // namespace QtControls
