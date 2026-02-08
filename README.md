# Document Editor Framework - Complete Design Patterns Implementation

## Overview
This is a comprehensive C++ framework for a structured document editor that implements **all 21 required design patterns** from the Gang of Four and additional architectural patterns. The framework serves as a foundation for building full-scale document editing applications.

## Compilation & Execution

```bash
g++ -std=c++17 -o document_editor structured_document_editor_framework.cpp
./document_editor
```

## Design Patterns Implemented

### Category 1: Creational Patterns (5/5)

1. **Singleton** - `ApplicationSettings`
   - Ensures only one instance of global settings exists
   - Manages font name, size, and paper size configuration

2. **Builder** - `DocumentBuilder`
   - Constructs complex `Document` objects step-by-step
   - Allows setting page size, margins, headers, and footers
   - Provides fluent interface for method chaining

3. **Factory Method** - `ElementFactory`
   - Creates various `DocumentElement` types (Paragraph, Image, Table, Section)
   - Centralizes object creation for easy extension

4. **Prototype** - `clone()` method
   - Every `DocumentElement` implements cloneable interface
   - Supports future Copy/Paste functionality
   - Deep cloning of composite structures

5. **Abstract Factory** (Implicit in ElementFactory)
   - Factory creates families of related objects

### Category 2: Structural Patterns (7/7)

6. **Composite** - `Section` class
   - Tree-like document structure
   - Sections can contain elements and sub-sections
   - Uniform treatment of individual and composite objects

7. **Decorator** - `BoldDecorator`, `ItalicDecorator`
   - Dynamically adds formatting to text elements
   - Wraps existing elements without modifying them
   - Stackable decorators for combined formatting

8. **Flyweight** - `CharacterFormatFactory`
   - Shares redundant character properties (Font, Size, Color)
   - Minimizes memory usage for repeated formatting
   - Demonstrates intrinsic vs extrinsic state separation

9. **Proxy** - `ImageProxy`
   - Virtual proxy for lazy image loading
   - Delays expensive file loading until needed
   - Optimizes performance by loading on demand

10. **Bridge** - `IRenderer` interface
    - Separates Document Model from Rendering Logic
    - Concrete renderers: `ConsoleRenderer`, `HTMLRenderer`
    - Allows independent variation of abstraction and implementation

11. **Facade** - `FileManagerFacade`
    - Simplifies complex file operations
    - Provides simple `.save()` and `.load()` interface
    - Hides serialization complexity

12. **Adapter** - `ShapeAdapter`
    - Adapts `LegacyShapeDrawer` to work with modern interface
    - Converts incompatible interfaces
    - Enables integration of external libraries

### Category 3: Behavioral Patterns (9+/9)

13. **Command** - `AddElementCommand`, `CommandHistory`
    - Encapsulates document modifications as objects
    - Enables Undo/Redo functionality
    - Maintains command history with undo/redo stacks

14. **Memento** - `DocumentMemento`
    - Saves document state snapshots
    - Works with Command pattern for undo/redo
    - Preserves encapsulation while saving state

15. **Observer** - `StatusBar` observing `Document`
    - Automatically updates when document changes
    - Maintains word count and element count
    - Loose coupling between subject and observers

16. **State** - `DraftState`, `ReviewState`, `PublishedState`
    - Document behavior changes based on current state
    - Editing permissions vary by state
    - Clean state transition management

17. **Strategy** - `IExportStrategy`
    - Interchangeable export algorithms: `ExportAsPDF`, `ExportAsMarkdown`
    - Runtime strategy switching
    - Easy to add new export formats

18. **Iterator** - `DocumentIterator`
    - Traverses composite document structure
    - Sequential access to all elements
    - Hides internal representation

19. **Visitor** - `IDocumentVisitor`
    - Performs operations across document elements
    - Concrete visitors: `WordCountVisitor`, `XMLExportVisitor`
    - Separates algorithms from object structure

20. **Template Method** - `DocumentValidator`
    - Defines validation algorithm skeleton
    - Subclasses implement specific validation steps
    - `BasicValidator` and `AdvancedValidator` implementations

21. **Chain of Responsibility** - `ClickHandler`, `KeyPressHandler`
    - Event handling chain
    - Passes requests through handler hierarchy
    - Decouples sender from receiver

### Bonus Patterns Implemented

22. **Mediator** - `UIMediator`
    - Coordinates complex UI component interactions
    - Centralizes communication logic
    - Reduces coupling between UI elements

23. **Interpreter** - `MacroInterpreter`
    - Interprets simple macro commands
    - Example: "BOLD ALL HEADINGS"
    - Demonstrates language processing

## Architecture Overview

### Core Components

```
ApplicationSettings (Singleton)
        ↓
DocumentBuilder → Document (Observable)
        ↓              ↓
   ElementFactory    StatusBar (Observer)
        ↓              ↓
  DocumentElement ← DocumentState
        ↓              ↓
   [Composite Tree]  State Implementations
        ↓
   IRenderer (Bridge)
        ↓
ConsoleRenderer / HTMLRenderer
```

### Class Hierarchy

```
DocumentElement (Abstract)
├── Paragraph
├── Image
├── Table
├── Section (Composite)
├── TextDecorator (Decorator Base)
│   ├── BoldDecorator
│   └── ItalicDecorator
├── ImageProxy (Proxy)
└── ShapeAdapter (Adapter)
```

## Key Features

### 1. Document Structure
- Hierarchical tree structure using Composite pattern
- Support for paragraphs, images, tables, and nested sections
- Element cloning for copy/paste operations

### 2. Rendering System
- Bridge pattern separates model from rendering
- Multiple renderer implementations (Console, HTML)
- Easy to add new renderers (PDF, GUI, etc.)

### 3. State Management
- Document lifecycle: Draft → Review → Published
- State-dependent behavior
- Clear transition management

### 4. Undo/Redo System
- Command pattern with history
- Memento for state snapshots
- Full operation reversal capability

### 5. Observer Pattern
- Automatic UI updates (StatusBar)
- Word count and element tracking
- Loose coupling between components

### 6. Formatting System
- Flyweight for memory-efficient character formatting
- Decorator for dynamic text styling
- Reusable format objects

### 7. Export System
- Strategy pattern for different export formats
- Currently supports: PDF, Markdown
- Easy to extend with new formats

### 8. Validation System
- Template Method for validation workflow
- Basic and Advanced validator implementations
- Extensible validation steps

## SOLID Principles Adherence

### Single Responsibility Principle (SRP)
- Each class has one clear responsibility
- Separate classes for rendering, validation, export, etc.

### Open/Closed Principle (OCP)
- Open for extension (new element types, renderers, strategies)
- Closed for modification (core interfaces stable)

### Liskov Substitution Principle (LSP)
- All DocumentElement subclasses can substitute base class
- All renderers are interchangeable

### Interface Segregation Principle (ISP)
- Focused interfaces (IRenderer, IDocumentVisitor, etc.)
- Clients depend only on methods they use

### Dependency Inversion Principle (DIP)
- High-level modules depend on abstractions
- Bridge, Strategy, and other patterns enforce this

## Usage Examples

### Creating a Document

```cpp
// Using Builder pattern
DocumentBuilder builder;
auto doc = builder.setPageSize("A4")
                 .setMargins(20, 20, 20, 20)
                 .setHeader("My Document")
                 .build();

// Using Factory pattern
doc->addElement(ElementFactory::createParagraph("Hello World"));
doc->addElement(ElementFactory::createImage("photo.jpg"));
doc->addElement(ElementFactory::createTable(3, 4));
```

### Adding Formatting

```cpp
// Using Decorator pattern
auto boldText = std::make_unique<BoldDecorator>(
    ElementFactory::createParagraph("Bold text")
);
doc->addElement(std::move(boldText));
```

### Rendering

```cpp
// Using Bridge pattern
ConsoleRenderer consoleRenderer;
doc->draw(&consoleRenderer);

HTMLRenderer htmlRenderer;
doc->draw(&htmlRenderer);
```

### Exporting

```cpp
// Using Strategy pattern
DocumentExporter exporter;
exporter.setStrategy(std::make_unique<ExportAsPDF>());
exporter.exportDocument(doc.get());
```

### Observing Changes

```cpp
// Using Observer pattern
StatusBar statusBar;
doc->attach(&statusBar);
doc->addElement(ElementFactory::createParagraph("New content"));
// StatusBar automatically updates
```

### State Management

```cpp
// Using State pattern
doc->edit(); // Works in Draft state
doc->setState(std::make_unique<PublishedState>());
doc->edit(); // Blocked in Published state
```

### Undo/Redo

```cpp
// Using Command & Memento patterns
CommandHistory history;
history.executeCommand(std::make_unique<AddElementCommand>(
    doc.get(), ElementFactory::createParagraph("Text")
));
history.undo();
history.redo();
```

## Extensibility

### Adding New Element Types
1. Create class inheriting from `DocumentElement`
2. Implement `draw()`, `clone()`, and `accept()` methods
3. Add factory method to `ElementFactory`
4. Update visitors if needed

### Adding New Renderers
1. Implement `IRenderer` interface
2. Provide rendering logic for all element types
3. Use with existing document structure via Bridge

### Adding New Export Formats
1. Implement `IExportStrategy` interface
2. Provide export logic
3. Swap strategy at runtime

### Adding New Validators
1. Inherit from `DocumentValidator`
2. Implement `checkSpelling()` and `checkGrammar()`
3. Template Method handles the workflow

## Testing the Implementation

Run the program to see all 21 patterns demonstrated:

```bash
./document_editor
```

The output shows:
- Pattern initialization and usage
- Pattern interactions
- Clear labeling of each pattern demonstration

## Design Pattern Benefits Demonstrated

### Flexibility
- Easy to add new element types, renderers, and strategies
- Runtime behavior changes via State and Strategy

### Maintainability
- Clear separation of concerns
- Each pattern addresses specific design challenge

### Scalability
- Composite structure supports arbitrary complexity
- Flyweight optimizes memory for large documents
- Proxy enables lazy loading for performance

### Testability
- Dependency injection via interfaces
- Mock objects possible for all abstractions
- Command pattern enables operation testing

## Future Enhancements

1. **Persistence**: Full JSON/XML serialization
2. **GUI**: Qt or wxWidgets renderer implementation
3. **Collaboration**: Multi-user editing with Observer
4. **Plugins**: Dynamic loading of new element types
5. **Scripting**: Enhanced Interpreter for macros
6. **Versioning**: Enhanced Memento for full history

## Conclusion

This framework demonstrates professional-grade software architecture using all major design patterns. It provides a solid foundation for building a production document editor while maintaining flexibility, maintainability, and extensibility.

All 21+ design patterns work together cohesively to create a robust, extensible system that follows SOLID principles and best practices in object-oriented design.

---

**Language**: C++17  
**Patterns**: 23 (21 required + 2 bonus)  
**Status**: Complete & Tested ✅

## Creator

Watchapon Wongapinya
Final project for Introduction to Computer Programming: Part 2