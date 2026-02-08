#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stack>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

// Forward declarations
class Document;
class DocumentElement;
class IRenderer;

// ==========================================================
// CATEGORY 1: CREATIONAL PATTERNS
// ==========================================================

// [SINGLETON] - Global Application Settings
class ApplicationSettings {
private:
    ApplicationSettings() : defaultFontSize(12), fontName("Arial"), paperSize("A4") {}
    static ApplicationSettings* instance;
public:
    int defaultFontSize;
    std::string fontName;
    std::string paperSize;

    static ApplicationSettings* getInstance() {
        if (!instance) instance = new ApplicationSettings();
        return instance;
    }
};
ApplicationSettings* ApplicationSettings::instance = nullptr;

// [BUILDER] - Complex Document Construction
class DocumentBuilder {
private:
    std::string pageSize;
    int marginTop, marginBottom, marginLeft, marginRight;
    std::string header;
    std::string footer;
public:
    DocumentBuilder() : pageSize("A4"), marginTop(20), marginBottom(20),
        marginLeft(20), marginRight(20), header(""), footer("") {
    }

    DocumentBuilder& setPageSize(const std::string& size) { pageSize = size; return *this; }
    DocumentBuilder& setMargins(int top, int bottom, int left, int right) {
        marginTop = top; marginBottom = bottom; marginLeft = left; marginRight = right;
        return *this;
    }
    DocumentBuilder& setHeader(const std::string& h) { header = h; return *this; }
    DocumentBuilder& setFooter(const std::string& f) { footer = f; return *this; }

    std::unique_ptr<Document> build();
};

// ==========================================================
// STRUCTURAL PATTERNS - Base Classes
// ==========================================================

// [BRIDGE] - Renderer Interface
class IRenderer {
public:
    virtual void renderText(const std::string& text, bool bold = false, bool italic = false) = 0;
    virtual void renderImage(const std::string& path) = 0;
    virtual void renderTable(int rows, int cols) = 0;
    virtual void startSection() = 0;
    virtual void endSection() = 0;
    virtual ~IRenderer() = default;
};

class ConsoleRenderer : public IRenderer {
public:
    void renderText(const std::string& text, bool bold, bool italic) override {
        std::string prefix = "";
        if (bold) prefix += "[BOLD]";
        if (italic) prefix += "[ITALIC]";
        std::cout << prefix << " " << text << std::endl;
    }
    void renderImage(const std::string& path) override {
        std::cout << "[IMAGE: " << path << "]" << std::endl;
    }
    void renderTable(int rows, int cols) override {
        std::cout << "[TABLE: " << rows << "x" << cols << "]" << std::endl;
    }
    void startSection() override { std::cout << "--- Section Start ---\n"; }
    void endSection() override { std::cout << "--- Section End ---\n"; }
};

class HTMLRenderer : public IRenderer {
public:
    void renderText(const std::string& text, bool bold, bool italic) override {
        std::string html = text;
        if (bold) html = "<strong>" + html + "</strong>";
        if (italic) html = "<em>" + html + "</em>";
        std::cout << "<p>" << html << "</p>\n";
    }
    void renderImage(const std::string& path) override {
        std::cout << "<img src=\"" << path << "\" />\n";
    }
    void renderTable(int rows, int cols) override {
        std::cout << "<table data-rows=\"" << rows << "\" data-cols=\"" << cols << "\"></table>\n";
    }
    void startSection() override { std::cout << "<section>\n"; }
    void endSection() override { std::cout << "</section>\n"; }
};

// [COMPOSITE] & [PROTOTYPE] - Document Element Base
class DocumentElement {
public:
    virtual void draw(IRenderer* renderer) = 0;
    virtual std::unique_ptr<DocumentElement> clone() const = 0;
    virtual void accept(class IDocumentVisitor* visitor) = 0;
    virtual std::string getType() const = 0;
    virtual ~DocumentElement() = default;
};

// [FLYWEIGHT] - Character Formatting (shared properties)
class CharacterFormat {
public:
    std::string fontName;
    int fontSize;
    std::string color;

    CharacterFormat(std::string font, int size, std::string col)
        : fontName(font), fontSize(size), color(col) {
    }
};

class CharacterFormatFactory {
private:
    std::map<std::string, std::shared_ptr<CharacterFormat>> formats;

    std::string getKey(const std::string& font, int size, const std::string& color) {
        return font + "_" + std::to_string(size) + "_" + color;
    }
public:
    std::shared_ptr<CharacterFormat> getFormat(const std::string& font, int size, const std::string& color) {
        std::string key = getKey(font, size, color);
        if (formats.find(key) == formats.end()) {
            formats[key] = std::make_shared<CharacterFormat>(font, size, color);
            std::cout << "[Flyweight] Created new format: " << key << std::endl;
        }
        return formats[key];
    }
};

// Concrete Elements
class Paragraph : public DocumentElement {
protected:
    std::string content;
    std::shared_ptr<CharacterFormat> format;
public:
    Paragraph(std::string text, std::shared_ptr<CharacterFormat> fmt = nullptr)
        : content(text), format(fmt) {
    }

    void draw(IRenderer* renderer) override {
        renderer->renderText(content);
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<Paragraph>(*this);
    }

    void accept(class IDocumentVisitor* visitor) override;

    std::string getType() const override { return "Paragraph"; }
    std::string getContent() const { return content; }
};

class Image : public DocumentElement {
protected:
    std::string imagePath;
public:
    Image(std::string path) : imagePath(path) {}

    void draw(IRenderer* renderer) override {
        renderer->renderImage(imagePath);
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<Image>(*this);
    }

    void accept(class IDocumentVisitor* visitor) override;

    std::string getType() const override { return "Image"; }
    std::string getPath() const { return imagePath; }
};

class Table : public DocumentElement {
protected:
    int rows, cols;
public:
    Table(int r, int c) : rows(r), cols(c) {}

    void draw(IRenderer* renderer) override {
        renderer->renderTable(rows, cols);
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<Table>(*this);
    }

    void accept(class IDocumentVisitor* visitor) override;

    std::string getType() const override { return "Table"; }
};

// [COMPOSITE] - Section that contains elements
class Section : public DocumentElement {
protected:
    std::vector<std::unique_ptr<DocumentElement>> children;
    std::string sectionName;
public:
    Section(std::string name = "") : sectionName(name) {}

    void add(std::unique_ptr<DocumentElement> el) {
        children.push_back(std::move(el));
    }

    void draw(IRenderer* renderer) override {
        renderer->startSection();
        for (auto& child : children) child->draw(renderer);
        renderer->endSection();
    }

    std::unique_ptr<DocumentElement> clone() const override {
        auto newSection = std::make_unique<Section>(sectionName);
        for (auto& child : children) {
            newSection->add(child->clone());
        }
        return newSection;
    }

    void accept(class IDocumentVisitor* visitor) override;

    std::string getType() const override { return "Section"; }

    const std::vector<std::unique_ptr<DocumentElement>>& getChildren() const {
        return children;
    }
};

// [FACTORY METHOD] - Element Factory
class ElementFactory {
public:
    static std::unique_ptr<DocumentElement> createParagraph(const std::string& text) {
        return std::make_unique<Paragraph>(text);
    }

    static std::unique_ptr<DocumentElement> createImage(const std::string& path) {
        return std::make_unique<Image>(path);
    }

    static std::unique_ptr<DocumentElement> createTable(int rows, int cols) {
        return std::make_unique<Table>(rows, cols);
    }

    static std::unique_ptr<DocumentElement> createSection(const std::string& name = "") {
        return std::make_unique<Section>(name);
    }
};

// [DECORATOR] - Text Formatting Decorators
class TextDecorator : public DocumentElement {
protected:
    std::unique_ptr<DocumentElement> wrappedElement;
public:
    TextDecorator(std::unique_ptr<DocumentElement> element)
        : wrappedElement(std::move(element)) {
    }

    std::string getType() const override { return wrappedElement->getType(); }
};

class BoldDecorator : public TextDecorator {
public:
    BoldDecorator(std::unique_ptr<DocumentElement> element)
        : TextDecorator(std::move(element)) {
    }

    void draw(IRenderer* renderer) override {
        // Simplified: just render with bold flag
        if (auto* para = dynamic_cast<Paragraph*>(wrappedElement.get())) {
            renderer->renderText(para->getContent(), true, false);
        }
        else {
            wrappedElement->draw(renderer);
        }
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<BoldDecorator>(wrappedElement->clone());
    }

    void accept(class IDocumentVisitor* visitor) override {
        wrappedElement->accept(visitor);
    }
};

class ItalicDecorator : public TextDecorator {
public:
    ItalicDecorator(std::unique_ptr<DocumentElement> element)
        : TextDecorator(std::move(element)) {
    }

    void draw(IRenderer* renderer) override {
        if (auto* para = dynamic_cast<Paragraph*>(wrappedElement.get())) {
            renderer->renderText(para->getContent(), false, true);
        }
        else {
            wrappedElement->draw(renderer);
        }
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<ItalicDecorator>(wrappedElement->clone());
    }

    void accept(class IDocumentVisitor* visitor) override {
        wrappedElement->accept(visitor);
    }
};

// [PROXY] - Virtual Proxy for Image Loading
class ImageProxy : public DocumentElement {
private:
    std::string imagePath;
    mutable std::unique_ptr<Image> realImage;

    void loadImage() const {
        if (!realImage) {
            std::cout << "[Proxy] Loading image: " << imagePath << std::endl;
            realImage = std::make_unique<Image>(imagePath);
        }
    }
public:
    ImageProxy(std::string path) : imagePath(path), realImage(nullptr) {}

    void draw(IRenderer* renderer) override {
        loadImage();
        realImage->draw(renderer);
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<ImageProxy>(imagePath);
    }

    void accept(class IDocumentVisitor* visitor) override;

    std::string getType() const override { return "ImageProxy"; }
};

// ==========================================================
// DOCUMENT CLASS
// ==========================================================

// [OBSERVER] - Observer Interface
class IDocumentObserver {
public:
    virtual void onDocumentChanged(Document* doc) = 0;
    virtual ~IDocumentObserver() = default;
};

// Document Class (Observable)
class Document {
private:
    std::unique_ptr<Section> rootSection;
    std::vector<IDocumentObserver*> observers;
    std::unique_ptr<class DocumentState> currentState;

    // Document properties from Builder
    std::string pageSize;
    int marginTop, marginBottom, marginLeft, marginRight;
    std::string header, footer;

public:
    Document();  // Implementation moved after DraftState is defined

    void setProperties(const std::string& ps, int mt, int mb, int ml, int mr,
        const std::string& h, const std::string& f) {
        pageSize = ps;
        marginTop = mt; marginBottom = mb; marginLeft = ml; marginRight = mr;
        header = h; footer = f;
    }

    void addElement(std::unique_ptr<DocumentElement> element) {
        rootSection->add(std::move(element));
        notifyObservers();
    }

    void draw(IRenderer* renderer) {
        rootSection->draw(renderer);
    }

    Section* getRootSection() { return rootSection.get(); }

    // Observer pattern
    void attach(IDocumentObserver* observer) {
        observers.push_back(observer);
    }

    void detach(IDocumentObserver* observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }

    void notifyObservers() {
        for (auto* observer : observers) {
            observer->onDocumentChanged(this);
        }
    }

    // State pattern
    void setState(std::unique_ptr<class DocumentState> state);
    void edit();
    class DocumentState* getState() { return currentState.get(); }
};

// Builder build method implementation
std::unique_ptr<Document> DocumentBuilder::build() {
    auto doc = std::make_unique<Document>();
    doc->setProperties(pageSize, marginTop, marginBottom, marginLeft, marginRight, header, footer);
    return doc;
}

// [OBSERVER] - Concrete Observer (StatusBar)
class StatusBar : public IDocumentObserver {
private:
    int wordCount;
    int elementCount;
public:
    StatusBar() : wordCount(0), elementCount(0) {}

    void onDocumentChanged(Document* doc) override {
        // Simple word count visitor could be used here
        elementCount = 0;
        wordCount = 0;
        // Count elements (simplified)
        countElements(doc->getRootSection());

        std::cout << "[StatusBar] Elements: " << elementCount << " | Words: " << wordCount << std::endl;
    }

    void countElements(Section* section) {
        for (auto& child : section->getChildren()) {
            elementCount++;
            if (auto* para = dynamic_cast<Paragraph*>(child.get())) {
                // Simple word count
                std::istringstream iss(para->getContent());
                std::string word;
                while (iss >> word) wordCount++;
            }
            if (auto* sec = dynamic_cast<Section*>(child.get())) {
                countElements(sec);
            }
        }
    }
};

// ==========================================================
// BEHAVIORAL PATTERNS
// ==========================================================

// [STATE] - Document States
class DocumentState {
public:
    virtual void edit(Document* doc) = 0;
    virtual std::string getStateName() const = 0;
    virtual ~DocumentState() = default;
};

class DraftState : public DocumentState {
public:
    void edit(Document* doc) override {
        std::cout << "[State] Editing in Draft mode - all changes allowed\n";
    }
    std::string getStateName() const override { return "Draft"; }
};

class ReviewState : public DocumentState {
public:
    void edit(Document* doc) override {
        std::cout << "[State] Document in Review - limited editing allowed\n";
    }
    std::string getStateName() const override { return "Review"; }
};

class PublishedState : public DocumentState {
public:
    void edit(Document* doc) override {
        std::cout << "[State] Document is Published - editing locked!\n";
    }
    std::string getStateName() const override { return "Published"; }
};

void Document::setState(std::unique_ptr<DocumentState> state) {
    currentState = std::move(state);
    std::cout << "[State] Document state changed to: " << currentState->getStateName() << std::endl;
}

void Document::edit() {
    currentState->edit(this);
}

// Document constructor implementation (after DraftState is defined)
Document::Document() : rootSection(std::make_unique<Section>("Root")),
pageSize("A4"), marginTop(20), marginBottom(20),
marginLeft(20), marginRight(20) {
    setState(std::make_unique<DraftState>());
}

// [MEMENTO] - Document State Snapshot
class DocumentMemento {
private:
    std::string state;
    std::string stateName;
public:
    DocumentMemento(const std::string& s, const std::string& sn)
        : state(s), stateName(sn) {
    }

    std::string getState() const { return state; }
    std::string getStateName() const { return stateName; }
};

// [COMMAND] - Command Pattern for Undo/Redo
class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() = default;
};

class AddElementCommand : public Command {
private:
    Document* document;
    std::unique_ptr<DocumentElement> element;
    std::unique_ptr<DocumentElement> clonedElement;  // Keep a clone for redo
    bool executed;
public:
    AddElementCommand(Document* doc, std::unique_ptr<DocumentElement> el)
        : document(doc), element(std::move(el)), executed(false) {
    }

    void execute() override {
        if (!executed) {
            std::cout << "[Command] Executing: Add Element\n";
            if (element) {
                clonedElement = element->clone();  // Keep clone for potential redo
                document->addElement(std::move(element));
            }
            else if (clonedElement) {
                element = clonedElement->clone();
                document->addElement(std::move(element));
            }
            executed = true;
        }
    }

    void undo() override {
        std::cout << "[Command] Undoing: Add Element (simplified - not removing)\n";
        // In real implementation, would remove the element
        executed = false;
    }
};

class CommandHistory {
private:
    std::stack<std::unique_ptr<Command>> undoStack;
    std::stack<std::unique_ptr<Command>> redoStack;
public:
    void executeCommand(std::unique_ptr<Command> cmd) {
        cmd->execute();
        undoStack.push(std::move(cmd));
        // Clear redo stack on new command
        while (!redoStack.empty()) redoStack.pop();
    }

    void undo() {
        if (!undoStack.empty()) {
            auto cmd = std::move(undoStack.top());
            undoStack.pop();
            cmd->undo();
            redoStack.push(std::move(cmd));
        }
        else {
            std::cout << "[Command] Nothing to undo\n";
        }
    }

    void redo() {
        if (!redoStack.empty()) {
            auto cmd = std::move(redoStack.top());
            redoStack.pop();
            cmd->execute();
            undoStack.push(std::move(cmd));
        }
        else {
            std::cout << "[Command] Nothing to redo\n";
        }
    }
};

// [STRATEGY] - Export Strategies
class IExportStrategy {
public:
    virtual void exportDocument(Document* doc) = 0;
    virtual ~IExportStrategy() = default;
};

class ExportAsPDF : public IExportStrategy {
public:
    void exportDocument(Document* doc) override {
        std::cout << "[Strategy] Exporting document as PDF...\n";
        // Simplified PDF export
        std::cout << "PDF export completed.\n";
    }
};

class ExportAsMarkdown : public IExportStrategy {
public:
    void exportDocument(Document* doc) override {
        std::cout << "[Strategy] Exporting document as Markdown...\n";
        // Simplified Markdown export
        std::cout << "Markdown export completed.\n";
    }
};

class DocumentExporter {
private:
    std::unique_ptr<IExportStrategy> strategy;
public:
    void setStrategy(std::unique_ptr<IExportStrategy> strat) {
        strategy = std::move(strat);
    }

    void exportDocument(Document* doc) {
        if (strategy) {
            strategy->exportDocument(doc);
        }
        else {
            std::cout << "[Exporter] No export strategy set!\n";
        }
    }
};

// [VISITOR] - Document Visitor Pattern
class IDocumentVisitor {
public:
    virtual void visitParagraph(Paragraph* para) = 0;
    virtual void visitImage(Image* img) = 0;
    virtual void visitTable(Table* table) = 0;
    virtual void visitSection(Section* section) = 0;
    virtual void visitImageProxy(ImageProxy* proxy) = 0;
    virtual ~IDocumentVisitor() = default;
};

// Implement accept methods
void Paragraph::accept(IDocumentVisitor* visitor) { visitor->visitParagraph(this); }
void Image::accept(IDocumentVisitor* visitor) { visitor->visitImage(this); }
void Table::accept(IDocumentVisitor* visitor) { visitor->visitTable(this); }
void ImageProxy::accept(IDocumentVisitor* visitor) { visitor->visitImageProxy(this); }
void Section::accept(IDocumentVisitor* visitor) {
    visitor->visitSection(this);
    for (auto& child : children) {
        child->accept(visitor);
    }
}

class WordCountVisitor : public IDocumentVisitor {
private:
    int wordCount;
public:
    WordCountVisitor() : wordCount(0) {}

    void visitParagraph(Paragraph* para) override {
        std::istringstream iss(para->getContent());
        std::string word;
        while (iss >> word) wordCount++;
    }

    void visitImage(Image* img) override { /* No words in image */ }
    void visitTable(Table* table) override { /* No words in table */ }
    void visitSection(Section* section) override { /* Process children */ }
    void visitImageProxy(ImageProxy* proxy) override { /* No words */ }

    int getWordCount() const { return wordCount; }
};

class XMLExportVisitor : public IDocumentVisitor {
private:
    std::ostringstream xml;
    int depth;

    std::string indent() { return std::string(depth * 2, ' '); }
public:
    XMLExportVisitor() : depth(0) { xml << "<?xml version=\"1.0\"?>\n"; }

    void visitParagraph(Paragraph* para) override {
        xml << indent() << "<paragraph>" << para->getContent() << "</paragraph>\n";
    }

    void visitImage(Image* img) override {
        xml << indent() << "<image src=\"" << img->getPath() << "\" />\n";
    }

    void visitTable(Table* table) override {
        xml << indent() << "<table />\n";
    }

    void visitSection(Section* section) override {
        xml << indent() << "<section>\n";
        depth++;
    }

    void visitImageProxy(ImageProxy* proxy) override {
        xml << indent() << "<image-proxy />\n";
    }

    std::string getXML() const { return xml.str(); }
};

// [ITERATOR] - Document Element Iterator
class DocumentIterator {
private:
    std::vector<DocumentElement*> elements;
    size_t position;

    void collectElements(Section* section) {
        for (auto& child : section->getChildren()) {
            elements.push_back(child.get());
            if (auto* sec = dynamic_cast<Section*>(child.get())) {
                collectElements(sec);
            }
        }
    }
public:
    DocumentIterator(Document* doc) : position(0) {
        collectElements(doc->getRootSection());
    }

    bool hasNext() const { return position < elements.size(); }

    DocumentElement* next() {
        if (hasNext()) {
            return elements[position++];
        }
        return nullptr;
    }

    void reset() { position = 0; }
};

// [TEMPLATE METHOD] - Document Validator
class DocumentValidator {
public:
    bool validate() {
        std::cout << "[Validator] Starting validation...\n";

        if (!checkStructure()) return false;
        if (!checkSpelling()) return false;
        if (!checkGrammar()) return false;

        std::cout << "[Validator] Validation passed!\n";
        return true;
    }

    virtual ~DocumentValidator() = default;

protected:
    virtual bool checkStructure() {
        std::cout << "[Validator] Checking structure... OK\n";
        return true;
    }

    virtual bool checkSpelling() = 0;  // Must be implemented
    virtual bool checkGrammar() = 0;   // Must be implemented
};

class BasicValidator : public DocumentValidator {
protected:
    bool checkSpelling() override {
        std::cout << "[Validator] Basic spell check... OK\n";
        return true;
    }

    bool checkGrammar() override {
        std::cout << "[Validator] Basic grammar check... OK\n";
        return true;
    }
};

class AdvancedValidator : public DocumentValidator {
protected:
    bool checkSpelling() override {
        std::cout << "[Validator] Advanced spell check with dictionary... OK\n";
        return true;
    }

    bool checkGrammar() override {
        std::cout << "[Validator] Advanced grammar check with AI... OK\n";
        return true;
    }
};

// [FACADE] - File Manager Facade
class FileManagerFacade {
public:
    void save(Document* doc, const std::string& path) {
        std::cout << "[Facade] Saving document to: " << path << std::endl;
        // Simplified: In real implementation, would serialize to JSON/XML
        std::ofstream file(path);
        if (file.is_open()) {
            file << "Document content saved\n";
            file.close();
            std::cout << "[Facade] Document saved successfully!\n";
        }
    }

    std::unique_ptr<Document> load(const std::string& path) {
        std::cout << "[Facade] Loading document from: " << path << std::endl;
        // Simplified: In real implementation, would deserialize from JSON/XML
        auto doc = std::make_unique<Document>();
        doc->addElement(ElementFactory::createParagraph("Loaded content"));
        std::cout << "[Facade] Document loaded successfully!\n";
        return doc;
    }
};

// [ADAPTER] - Legacy Shape Drawer Adapter
class LegacyShapeDrawer {
public:
    void drawRectangle(int x, int y, int width, int height) {
        std::cout << "[Legacy] Drawing rectangle at (" << x << "," << y
            << ") size: " << width << "x" << height << std::endl;
    }
};

class ShapeAdapter : public DocumentElement {
private:
    LegacyShapeDrawer legacyDrawer;
    int x, y, width, height;
public:
    ShapeAdapter(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}

    void draw(IRenderer* renderer) override {
        std::cout << "[Adapter] Adapting legacy shape to modern interface...\n";
        legacyDrawer.drawRectangle(x, y, width, height);
    }

    std::unique_ptr<DocumentElement> clone() const override {
        return std::make_unique<ShapeAdapter>(x, y, width, height);
    }

    void accept(IDocumentVisitor* visitor) override {
        // Adapter doesn't need visitor
    }

    std::string getType() const override { return "Shape"; }
};

// [CHAIN OF RESPONSIBILITY] - Event Handler Chain (Optional)
class EventHandler {
protected:
    EventHandler* nextHandler;
public:
    EventHandler() : nextHandler(nullptr) {}

    void setNext(EventHandler* next) { nextHandler = next; }

    virtual void handleEvent(const std::string& event) {
        if (nextHandler) {
            nextHandler->handleEvent(event);
        }
    }

    virtual ~EventHandler() = default;
};

class ClickHandler : public EventHandler {
public:
    void handleEvent(const std::string& event) override {
        if (event == "click") {
            std::cout << "[Chain] Click handled!\n";
        }
        else {
            EventHandler::handleEvent(event);
        }
    }
};

class KeyPressHandler : public EventHandler {
public:
    void handleEvent(const std::string& event) override {
        if (event == "keypress") {
            std::cout << "[Chain] Keypress handled!\n";
        }
        else {
            EventHandler::handleEvent(event);
        }
    }
};

// [MEDIATOR] - UI Mediator (Optional)
class UIMediator {
private:
    Document* document;
public:
    UIMediator(Document* doc) : document(doc) {}

    void onMenuClick(const std::string& menu) {
        std::cout << "[Mediator] Menu '" << menu << "' clicked, coordinating UI...\n";
        if (menu == "save") {
            FileManagerFacade facade;
            facade.save(document, "document.txt");
        }
    }

    void onButtonClick(const std::string& button) {
        std::cout << "[Mediator] Button '" << button << "' clicked\n";
    }
};

// [INTERPRETER] - Simple Macro System (Advanced/Optional)
class Expression {
public:
    virtual void interpret(Document* doc) = 0;
    virtual ~Expression() = default;
};

class BoldAllHeadingsExpression : public Expression {
public:
    void interpret(Document* doc) override {
        std::cout << "[Interpreter] Executing: BOLD ALL HEADINGS\n";
        // Simplified: would traverse and bold all headings
    }
};

class MacroInterpreter {
public:
    void execute(const std::string& command, Document* doc) {
        std::cout << "[Interpreter] Parsing command: " << command << std::endl;

        if (command == "BOLD ALL HEADINGS") {
            BoldAllHeadingsExpression expr;
            expr.interpret(doc);
        }
        else {
            std::cout << "[Interpreter] Unknown command\n";
        }
    }
};

// ==========================================================
// MAIN DEMONSTRATION
// ==========================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "DOCUMENT EDITOR FRAMEWORK DEMO\n";
    std::cout << "========================================\n\n";

    // 1. SINGLETON - Application Settings
    std::cout << "--- 1. SINGLETON ---\n";
    auto settings = ApplicationSettings::getInstance();
    std::cout << "Default Font: " << settings->fontName << ", Size: " << settings->defaultFontSize << "\n\n";

    // 2. BUILDER - Build complex document
    std::cout << "--- 2. BUILDER ---\n";
    DocumentBuilder builder;
    auto doc = builder.setPageSize("A4")
        .setMargins(20, 20, 20, 20)
        .setHeader("My Document")
        .setFooter("Page 1")
        .build();
    std::cout << "Document built with custom settings\n\n";

    // 3. FACTORY METHOD - Create elements
    std::cout << "--- 3. FACTORY METHOD ---\n";
    doc->addElement(ElementFactory::createParagraph("Introduction paragraph"));
    doc->addElement(ElementFactory::createParagraph("This is the second paragraph"));
    std::cout << "Elements created via factory\n\n";

    // 4. PROTOTYPE - Clone elements
    std::cout << "--- 4. PROTOTYPE ---\n";
    auto originalPara = ElementFactory::createParagraph("Original text");
    auto clonedPara = originalPara->clone();
    std::cout << "Paragraph cloned successfully\n\n";

    // 5. COMPOSITE - Section hierarchy
    std::cout << "--- 5. COMPOSITE ---\n";
    auto section = ElementFactory::createSection("Chapter 1");
    auto sectionPtr = dynamic_cast<Section*>(section.get());
    sectionPtr->add(ElementFactory::createParagraph("Chapter 1 content"));
    doc->addElement(std::move(section));
    std::cout << "Composite section added\n\n";

    // 6. DECORATOR - Bold and Italic
    std::cout << "--- 6. DECORATOR ---\n";
    auto boldPara = std::make_unique<BoldDecorator>(
        ElementFactory::createParagraph("Bold text")
    );
    auto italicPara = std::make_unique<ItalicDecorator>(
        ElementFactory::createParagraph("Italic text")
    );
    doc->addElement(std::move(boldPara));
    doc->addElement(std::move(italicPara));
    std::cout << "Decorated paragraphs added\n\n";

    // 7. FLYWEIGHT - Character formatting
    std::cout << "--- 7. FLYWEIGHT ---\n";
    CharacterFormatFactory formatFactory;
    auto format1 = formatFactory.getFormat("Arial", 12, "Black");
    auto format2 = formatFactory.getFormat("Arial", 12, "Black"); // Reuses same object
    std::cout << "Format objects shared: " << (format1 == format2 ? "Yes" : "No") << "\n\n";

    // 8. PROXY - Lazy image loading
    std::cout << "--- 8. PROXY ---\n";
    auto imageProxy = std::make_unique<ImageProxy>("photo.jpg");
    std::cout << "Image proxy created (not loaded yet)\n";
    doc->addElement(std::move(imageProxy));
    std::cout << "\n";

    // 9. BRIDGE - Render with different renderers
    std::cout << "--- 9. BRIDGE ---\n";
    ConsoleRenderer consoleRenderer;
    HTMLRenderer htmlRenderer;

    std::cout << "Console Rendering:\n";
    doc->draw(&consoleRenderer);

    std::cout << "\nHTML Rendering:\n";
    auto simplePara = ElementFactory::createParagraph("HTML test");
    simplePara->draw(&htmlRenderer);
    std::cout << "\n";

    // 10. FACADE - File operations
    std::cout << "--- 10. FACADE ---\n";
    FileManagerFacade fileManager;
    fileManager.save(doc.get(), "mydocument.txt");
    std::cout << "\n";

    // 11. ADAPTER - Legacy shape integration
    std::cout << "--- 11. ADAPTER ---\n";
    auto shape = std::make_unique<ShapeAdapter>(10, 20, 100, 50);
    std::cout << "Demonstrating adapter:\n";
    ConsoleRenderer adapterRenderer;
    shape->draw(&adapterRenderer);  // Demonstrate adapter in action
    doc->addElement(std::move(shape));
    std::cout << "\n";

    // 12. OBSERVER - Status bar
    std::cout << "--- 12. OBSERVER ---\n";
    StatusBar statusBar;
    doc->attach(&statusBar);
    doc->addElement(ElementFactory::createParagraph("Trigger observer update"));
    std::cout << "\n";

    // 13. STATE - Document states
    std::cout << "--- 13. STATE ---\n";
    doc->edit();
    doc->setState(std::make_unique<ReviewState>());
    doc->edit();
    doc->setState(std::make_unique<PublishedState>());
    doc->edit();
    std::cout << "\n";

    // 14. COMMAND - Undo/Redo
    std::cout << "--- 14. COMMAND & MEMENTO ---\n";
    CommandHistory history;
    history.executeCommand(std::make_unique<AddElementCommand>(
        doc.get(), ElementFactory::createParagraph("Command pattern test")
    ));
    history.undo();
    history.redo();
    std::cout << "\n";

    // 15. STRATEGY - Export strategies
    std::cout << "--- 15. STRATEGY ---\n";
    DocumentExporter exporter;
    exporter.setStrategy(std::make_unique<ExportAsPDF>());
    exporter.exportDocument(doc.get());
    exporter.setStrategy(std::make_unique<ExportAsMarkdown>());
    exporter.exportDocument(doc.get());
    std::cout << "\n";

    // 16. VISITOR - Word count and XML export
    std::cout << "--- 16. VISITOR ---\n";
    WordCountVisitor wordCounter;
    doc->getRootSection()->accept(&wordCounter);
    std::cout << "Total words (via visitor): " << wordCounter.getWordCount() << std::endl;

    XMLExportVisitor xmlExporter;
    doc->getRootSection()->accept(&xmlExporter);
    std::cout << "XML Export:\n" << xmlExporter.getXML() << "\n";

    // 17. ITERATOR - Traverse document
    std::cout << "--- 17. ITERATOR ---\n";
    DocumentIterator iterator(doc.get());
    std::cout << "Iterating through all elements:\n";
    int count = 0;
    while (iterator.hasNext()) {
        auto* element = iterator.next();
        std::cout << "  Element " << ++count << ": " << element->getType() << std::endl;
    }
    std::cout << "\n";

    // 18. TEMPLATE METHOD - Validation
    std::cout << "--- 18. TEMPLATE METHOD ---\n";
    BasicValidator basicValidator;
    basicValidator.validate();

    AdvancedValidator advValidator;
    advValidator.validate();
    std::cout << "\n";

    // 19. CHAIN OF RESPONSIBILITY - Event handling
    std::cout << "--- 19. CHAIN OF RESPONSIBILITY ---\n";
    ClickHandler clickHandler;
    KeyPressHandler keyHandler;
    clickHandler.setNext(&keyHandler);

    clickHandler.handleEvent("click");
    clickHandler.handleEvent("keypress");
    std::cout << "\n";

    // 20. MEDIATOR - UI coordination
    std::cout << "--- 20. MEDIATOR ---\n";
    UIMediator mediator(doc.get());
    mediator.onMenuClick("save");
    mediator.onButtonClick("bold");
    std::cout << "\n";

    // 21. INTERPRETER - Macro system
    std::cout << "--- 21. INTERPRETER ---\n";
    MacroInterpreter interpreter;
    interpreter.execute("BOLD ALL HEADINGS", doc.get());
    std::cout << "\n";

    std::cout << "========================================\n";
    std::cout << "ALL 21 DESIGN PATTERNS DEMONSTRATED!\n";
    std::cout << "========================================\n";

    return 0;
}