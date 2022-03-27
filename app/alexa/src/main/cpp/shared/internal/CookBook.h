#ifndef ACSDKMANUFACTORY_INTERNAL_COOKBOOK_H_
#define ACSDKMANUFACTORY_INTERNAL_COOKBOOK_H_

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <shared/Annotated.h>
#include "AbstractPointerCache.h"
#include "PointerCache.h"
#include "TypeIndex.h"
#include "CookBook_imp.h"

namespace alexaClientSDK {
namespace acsdkManufactory {
namespace internal {

// Forward declaration
class RuntimeManufactory;

/**
 * CookBook is a collection of recipes for creating instances.
 */
class CookBook {
public:
    /**
     * Constructor.
     */
    CookBook();

    /**
     * Add a factory that returns a @c std::unique_ptr value.
     *
     * @tparam Type The full type (including std::unique_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addUniqueFactory(std::function<std::unique_ptr<Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c std::unique_ptr value.
     *
     * @tparam Type The full type (including std::unique_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addUniqueFactory(std::unique_ptr<Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c primary value (i.e. a value
     * that must always be instantiated before all others).  If multiple 'primary' factories
     * are added, they will be instantiated in an arbitrary order - other than that required
     * by any dependencies.
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addPrimaryFactory(std::function<std::shared_ptr<Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c primary value (i.e. a value
     * that must always be instantiated before all others).  If multiple 'primary' factories
     * are added, they will be instantiated in an arbitrary order - other than that required
     * by any dependencies.
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addPrimaryFactory(std::function<Annotated<Annotation, Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c primary value (i.e. a value
     * that must always be instantiated before all others).  If multiple 'primary' factories
     * are added, they will be instantiated in an arbitrary order - other than that required
     * by any dependencies.
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addPrimaryFactory(std::shared_ptr<Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c primary value (i.e. a value
     * that must always be instantiated before all others).  If multiple 'primary' factories
     * are added, they will be instantiated in an arbitrary order - other than that required
     * by any dependencies.
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addPrimaryFactory(Annotated<Annotation, Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c required value (i.e. a value
     * that must always be instantiated).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addRequiredFactory(std::function<std::shared_ptr<Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c required value (i.e. a value
     * that must always be instantiated).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addRequiredFactory(std::function<Annotated<Annotation, Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c required value (i.e. a value
     * that must always be instantiated).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addRequiredFactory(std::shared_ptr<Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c required value (i.e. a value
     * that must always be instantiated).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addRequiredFactory(Annotated<Annotation, Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c retained value (i.e. a value
     * that must be retained once instantiated).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addRetainedFactory(std::function<std::shared_ptr<Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c retained value (i.e. a value
     * that must be retained once instantiated).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addRetainedFactory(std::function<Annotated<Annotation, Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c std::shared_ptr<> to a @c retained value (i.e. a value
     * that must be retained once instantiated).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addRetainedFactory(std::shared_ptr<Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c Annotated<> pointer to a @c retained value (i.e. a value
     * that must be retained once instantiated).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addRetainedFactory(Annotated<Annotation, Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c std::shared_ptr<> to an @c unloadable value (i.e. a value
     * that may be released when no longer referenced).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addUnloadableFactory(std::function<std::shared_ptr<Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c Annotated<> pointer to an @c unloadable value (i.e. a value
     * that may be released when no longer referenced).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addUnloadableFactory(std::function<Annotated<Annotation, Type>(Dependencies...)> factory);

    /**
     * Add a factory that returns a @c std::shared_ptr<> to an @c unloadable value (i.e. a value
     * that may be released when no longer referenced).
     *
     * @tparam Type The full type (including std::shared_ptr<>) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Type, typename... Dependencies>
    CookBook& addUnloadableFactory(std::shared_ptr<Type> (*factory)(Dependencies...));

    /**
     * Add a factory that returns a @c Annotated<> pointer to an @c unloadable value (i.e. a value
     * that may be released when no longer referenced).
     *
     * @tparam Annotation The type used to differentiate the returned instance of @c Type.
     * @tparam Type The full type (including Annotated) returned by the factory.
     * @tparam Dependencies The dependencies (arguments) of the factory to be added.
     * @param factory The factory to be added.
     * @return This @c CookBook.
     */
    template <typename Annotation, typename Type, typename... Dependencies>
    CookBook& addUnloadableFactory(Annotated<Annotation, Type> (*factory)(Dependencies...));

    /**
     * Declare support for providing instances of an type from an already existing instance.  Such instances
     * are assumed to be "retained" for the lifecycle of this @c CookBook.
     *
     * @tparam Type The type of interface supported.
     * @param instance The instance to be served up.
     * @return This @c CookBook instance, with the added factory.
     */
    template <typename Type>
    CookBook& addInstance(const Type& instance);

    /**
     * Merge the contents of a @c CookBook into this @c CookBook.
     *
     * If the incoming contents conflict (e.g. specifies a different factory for the same Type) this @c CookBook
     * will be marked invalid and will fail all further operations.
     *
     * @param cookBook The @c CookBook to merge into this @c CookBook.
     * @return This @c CookBook instance, merged with the contents of @c cookBook.
     */
    CookBook& addCookBook(const CookBook& cookBook);

    /**
     * Verify that this CookBook instance is complete (all Imports are satisfied, there are no cyclic dependencies,
     * and the the CookBook is otherwise valid).  If this check fails, this CookBook will be marked invalid.
     *
     * @return Whether this CookBook is valid.
     */
    bool checkCompleteness();

    /**
     * Perform get<Type>() for each Required<Type> registered with this CookBook.
     *
     * @param runtimeManufactory The @c RuntimeManufactory from which to retrieve the Required<Type>s and
     * any of their dependencies.
     * @return Whether all get<Type>() operations were successful.
     */
    bool doRequiredGets(RuntimeManufactory& runtimeManufactory);

    /**
     * Create a new instance @c Type and return it via @c std::unique_ptr<>.
     *
     * @tparam Type The type of value to instantiate.
     * @param runtimeManufactory The RuntimeManufactory to use to acquire dependencies of the new instance.
     * @return A @c std::unique_ptr<Type> to the new instance of Type.
     */
    template <typename Type>
    std::unique_ptr<Type> createUniquePointer(RuntimeManufactory& runtimeManufactory);

    /**
     * Create a @c PointerCache<Type> for the specified Type.
     *
     * @tparam Type The @c Type of object to be cached.
     * @return A new @c PointerCache<Type> instance.
     */
    template <typename Type>
    std::unique_ptr<PointerCache<Type>> createPointerCache();

private:
    /**
     * The base class for 'recipes' for creating instances.
     */
    class AbstractRecipe {
    public:
        /**
         * Destructor
         */
        virtual ~AbstractRecipe() = default;

        /**
         * Get the type of instance generated by this recipe.
         *
         * @return The type of instance generated by this recipe.
         */
        virtual TypeIndex getValueType() const = 0;

        /**
         * The type of this instance of @c AbstractRecipe.
         *
         * @return The type of this instance of @c AbstractRecipe.
         */
        virtual TypeIndex getRecipeType() const = 0;

        /**
         * Is this instance of @c AbstractRecipe equivalent to the specified recipe.
         *
         * @param Recipe The @c Recipe to compare with.
         *
         * @return Whether this instance of @c AbstractRecipe equivalent to the specified recipe.
         */
        virtual bool isEquivalent(const std::shared_ptr<AbstractRecipe>& recipe) const = 0;

        /**
         * Get the start of a vector enumerating the dependencies of the interface this Recipe creates.
         *
         * @return The start of a vector enumerating the dependencies of the interface this Recipe creates.
         */
        std::vector<TypeIndex>::const_iterator begin() const;

        /**
         * Get the end of a vector enumerating the dependencies of the interface this Recipe creates.
         *
         * @return The end of a vector enumerating the dependencies of the interface this Recipe creates.
         */
        std::vector<TypeIndex>::const_iterator end() const;

    protected:
        /// a vector enumerating the dependencies of the interface this @c AbstractRecipe creates.
        std::vector<TypeIndex> m_dependencies;
    };

    /**
     * The base class for recipes used to create instances of @c Type and return them via std::unique_ptr<Type>.
     *
     * @tparam Type The Type of instance created by this recipe.
     */
    template <typename Type>
    class UniquePointerRecipe : public AbstractRecipe {
    public:
        /// @name AbstractRecipe methods.
        /// @{
        TypeIndex getValueType() const override;
        /// @}

        /**
         * Create a new instance of @c Type and return it via @c std::unique_ptr<Type>.
         *
         * @param runtimeManufactory The @c RuntimeManufactory to use to acquire any dependencies of the
         * factory used to create istances of @c Type.
         * @return A new instance of @c Type returned via @c std::unique_ptr<Type>.
         */
        virtual std::unique_ptr<Type> createUniquePointer(RuntimeManufactory& runtimeManufactory) const = 0;
    };

    template <typename Type, typename... Dependencies>
    class UniquePointerFunctionRecipe : public UniquePointerRecipe<Type> {
    public:
        UniquePointerFunctionRecipe(std::function<std::unique_ptr<Type>(Dependencies...)> function);
        TypeIndex getRecipeType() const override;
        bool isEquivalent(const std::shared_ptr<AbstractRecipe>& recipe) const override;
        std::unique_ptr<Type> createUniquePointer(RuntimeManufactory& runtimeManufactory) const override;
    private:
        std::function<std::unique_ptr<Type>(Dependencies...)> m_function;
    };
    template <typename Type, typename... Dependencies> class UniquePointerFactoryRecipe : public UniquePointerRecipe<Type> {
    public:
        UniquePointerFactoryRecipe(std::unique_ptr<Type> (*factory)(Dependencies...));
        TypeIndex getRecipeType() const override;
        bool isEquivalent(const std::shared_ptr<AbstractRecipe>& recipe) const override;
        std::unique_ptr<Type> createUniquePointer(RuntimeManufactory& runtimeManufactory) const override;
    private:
        std::unique_ptr<Type> (*m_factory)(Dependencies...);
    };
    template <typename Type> class SharedPointerRecipe : public AbstractRecipe {
    public:
        virtual std::unique_ptr<PointerCache<Type>> createSharedPointerCache() const;
        TypeIndex getValueType() const override;
    };
    template <typename CacheType, typename Type, typename... Dependencies> class SharedPointerFunctionRecipe : public SharedPointerRecipe<Type> {
    public:
        SharedPointerFunctionRecipe(std::function<Type(Dependencies...)> function);
        std::unique_ptr<PointerCache<Type>> createSharedPointerCache() const override;
        TypeIndex getRecipeType() const override;
        bool isEquivalent(const std::shared_ptr<AbstractRecipe>& Recipe) const override;
    private:
        std::function<Type(Dependencies...)> m_function;
    };
    template <typename CacheType, typename Type, typename... Dependencies> class SharedPointerFactoryRecipe : public SharedPointerRecipe<Type> {
    public:
        SharedPointerFactoryRecipe(Type (*factory)(Dependencies...));
        std::unique_ptr<PointerCache<Type>> createSharedPointerCache() const override;
        TypeIndex getRecipeType() const override;
        bool isEquivalent(const std::shared_ptr<AbstractRecipe>& Recipe) const override;
    private:
        Type (*m_factory)(Dependencies...);
    };
    template <typename Type> class SharedPointerInstanceRecipe : public SharedPointerRecipe<Type> {
    public:
        SharedPointerInstanceRecipe(const Type& instance);
        std::unique_ptr<PointerCache<Type>> createSharedPointerCache() const override;
        TypeIndex getRecipeType() const override;
        bool isEquivalent(const std::shared_ptr<AbstractRecipe>& Recipe) const override;
    private:
        Type m_instance;
    };
    template <typename Type, typename... Dependencies> class RequiredPointerCache : public PointerCache<Type> {
    public:
        RequiredPointerCache(std::function<Type(Dependencies...)> factory);
        Type get(RuntimeManufactory& runtimeManufactory) override;
    private:
        Type m_value;
        std::function<Type(Dependencies...)> m_factory;
    };
    template <typename Type, typename... Dependencies> class RetainedPointerCache : public PointerCache<Type> {
    public:
        RetainedPointerCache(std::function<Type(Dependencies...)> factory);
        Type get(RuntimeManufactory& runtimeManufactory) override;
    private:
        Type m_value;
        std::function<Type(Dependencies...)> m_factory;
    };
    template <typename Type, typename... Dependencies> class UnloadablePointerCache : public PointerCache<Type> {
    public:
        UnloadablePointerCache(std::function<Type(Dependencies...)> factory);
        Type get(RuntimeManufactory& runtimeManufactory) override;
    private:
        std::weak_ptr<typename Type::element_type> m_value;
        std::function<Type(Dependencies...)> m_factory;
    };
    template <typename Type> class InstancePointerCache : public PointerCache<Type> {
    public:
        InstancePointerCache(Type instance);
        Type get(RuntimeManufactory& runtimeManufactory) override;
    private:
        Type m_instance;
    };
    using GetWrapper = bool (*)(RuntimeManufactory& runtimeManufactory);
    template <typename Type, typename... Dependencies> using InstanceGetter = std::function<Type(Dependencies...)>;
    template <typename RecipeType, typename ResultType, typename... Dependencies>
    CookBook& addFunctionRecipe(std::function<ResultType(Dependencies...)> function);
    template <typename RecipeType, typename ResultType, typename... Dependencies> CookBook& addFactoryRecipe(ResultType (*factory)(Dependencies...));
    bool checkForCyclicDependencies();
    bool checkIsValid(const char* functionName) const;
    void markInvalid(const std::string& event, const std::string& reason, const std::string& type = "");
    template <typename Type, typename... Dependencies>
    static Type invokeWithDependencies(RuntimeManufactory& runtimeManufactory, std::function<Type(Dependencies...)> function);
    template <typename Type, typename FunctionType, typename... Dependencies>
    static Type innerInvokeWithDependencies(FunctionType function, Dependencies... dependencies);
    template <typename First, typename... Remainder> static bool checkNonNull(const First& first, const Remainder&... remainder);
    static bool checkNonNull();
    static std::string getLoggerTag();
    bool m_isValid;
    std::unordered_map<TypeIndex, std::shared_ptr<AbstractRecipe>> m_recipes;
    std::unordered_set<GetWrapper> m_primaryGets;
    std::unordered_set<GetWrapper> m_requiredGets;
};
}
}
}
#endif