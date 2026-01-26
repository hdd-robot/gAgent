# Corrections de Compilation

## Date: 2025-12-13
## Problèmes Résolus

### 1. Include manquant pour waitpid()

**Erreur**:
```
error: 'waitpid' was not declared in this scope
```

**Solution**: Ajout de `#include <sys/wait.h>` dans Agent.cpp

---

### 2. Avertissement Boost placeholders

**Avertissement**:
```
warning: The practice of declaring the Bind placeholders (_1, _2, ...) 
in the global namespace is deprecated.
```

**Solution**: Ajout de `#define BOOST_BIND_GLOBAL_PLACEHOLDERS` avant les includes boost dans Agent.hpp

---

### 3. Standard C++11 au lieu de C++17

**Problème**: src_agent/CMakeLists.txt était configuré pour C++11

**Solution**: Mise à jour vers C++17
```cmake
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

### 4. std::make_unique non disponible

**Erreur**:
```
error: 'make_unique' is not a member of 'std'
note: 'std::make_unique' is only available from C++14 onwards
```

**Solution**: Remplacement de `std::make_unique<Impl>()` par `new Impl()` 
dans CommunicationManager.cpp (compatible avec C++11+)

---

### 5. Bibliothèque libconfig++ non linkée

**Erreur**:
```
undefined reference to `libconfig::Config::readFile(char const*)'
```

**Solution**: Ajout de `config++` dans les LIBS_TO_LINK:
- src_agent/CMakeLists.txt
- examples/CMakeLists.txt

---

## Résultat Final

✅ Compilation réussie sans erreurs
✅ Bibliothèque libgagent.so créée
✅ Exemples compilés
✅ Tests compilés

Tous les avertissements restants sont mineurs et n'affectent pas le fonctionnement.
