#pragma once
#include <QString>
#include <QJsonObject>

class TranslationRequest {
public:
    TranslationRequest();
    explicit TranslationRequest(const QString& text);
    
    // Getters
    QString getText() const;
    QString getSourceLanguage() const;
    QString getTargetLanguage() const;
    bool isAutoDetect() const;
    
    // Setters
    void setText(const QString& text);
    void setSourceLanguage(const QString& language);
    void setTargetLanguage(const QString& language);
    void setAutoDetect(bool autoDetect);
    
    // JSON serialization
    QJsonObject toJson() const;
    static TranslationRequest fromJson(const QJsonObject& json);
    
    // Validation
    bool isValid() const;
    QString getError() const;
    
private:
    void validate();
    
    QString m_text;
    QString m_sourceLanguage;
    QString m_targetLanguage;
    bool m_autoDetect;
    QString m_error;
};
