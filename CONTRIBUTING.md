# Contributing to gAgent

Thank you for your interest in contributing to gAgent! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help create a welcoming environment for all contributors

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/gAgent.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes
6. Commit with clear messages
7. Push to your fork
8. Create a Pull Request

## Development Setup

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake qtbase5-dev libboost-all-dev libconfig++-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake qt5-qtbase-devel boost-devel libconfig-devel
```

### Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
make -j$(nproc)
```

## Coding Standards

### C++ Guidelines

- **Standard**: C++17 or later
- **Formatting**: Use `.clang-format` (run `clang-format -i file.cpp`)
- **Naming**:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Variables: `snake_case` for locals, `camelCase` for members
  - Constants: `UPPER_SNAKE_CASE`
  - Namespaces: lowercase

### Best Practices

1. **Memory Management**
   - Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
   - Avoid raw `new`/`delete`
   - Follow RAII principles

2. **Error Handling**
   - Use exceptions for exceptional cases
   - Return `std::optional` for nullable returns
   - Document error conditions

3. **Thread Safety**
   - Document thread-safety guarantees
   - Use `std::mutex`, `std::lock_guard`
   - Avoid shared mutable state

4. **Documentation**
   - Use Doxygen comments for public APIs
   - Include examples in documentation
   - Keep comments up-to-date

### Example Code Style

```cpp
/**
 * @brief Brief description of the class
 * 
 * Detailed description of what this class does,
 * its purpose, and how to use it.
 */
class MyAgent : public Agent {
public:
    /**
     * @brief Constructor
     * @param name Agent name
     */
    explicit MyAgent(const std::string& name);
    
    /**
     * @brief Initialize the agent
     * @return true if initialization succeeds
     */
    bool initialize();
    
private:
    std::string agent_name_;
    std::unique_ptr<Behaviour> behaviour_;
};
```

## Testing

### Writing Tests

- Place tests in `test_agent/` directory
- Use descriptive test names
- Test both success and failure cases
- Aim for high code coverage

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Pull Request Process

1. **Before Submitting**
   - Ensure code compiles without warnings
   - Run all tests
   - Update documentation if needed
   - Format code with clang-format

2. **PR Description**
   - Describe what changes were made
   - Explain why the changes are needed
   - Reference related issues

3. **Review Process**
   - Address reviewer feedback
   - Keep commits clean and logical
   - Rebase on master if needed

## Commit Messages

Follow conventional commits format:

```
type(scope): subject

body

footer
```

**Types**: feat, fix, docs, style, refactor, test, chore

**Examples**:
```
feat(agent): add support for delayed behaviors

fix(messaging): resolve race condition in message queue

docs(readme): update installation instructions
```

## Areas for Contribution

### High Priority

- [ ] Migrate to smart pointers
- [ ] Add comprehensive unit tests
- [ ] Improve error handling
- [ ] Add logging framework
- [ ] Thread safety improvements

### Medium Priority

- [ ] Performance optimizations
- [ ] Extended FIPA-ACL support
- [ ] Better documentation
- [ ] More example agents
- [ ] Docker support

### Good First Issues

- Documentation improvements
- Code formatting fixes
- Adding code comments
- Simple bug fixes
- Test additions

## Questions?

- Open an issue for bugs or feature requests
- Use GitHub Discussions for questions
- Check existing issues before creating new ones

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
