/**
 * Forward declaration of the function that will return the GType of
 * the Value implementation. Not used in this program
 */
 
GType disnix_object_get_type (void);

/* 
 * Macro for the above. It is common to define macros using the
 * naming convention (seen below) for all GType implementations,
 * and that's why we're going to do that here as well.
 */
#define DISNIX_TYPE_OBJECT              (disnix_object_get_type ())
#define DISNIX_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), DISNIX_TYPE_OBJECT, DisnixObject))
#define DISNIX_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), DISNIX_TYPE_OBJECT, DisnixObjectClass))
#define DISNIX_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), DISNIX_TYPE_OBJECT))
#define DISNIX_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), DISNIX_TYPE_OBJECT))
#define DISNIX_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), DISNIX_TYPE_OBJECT, DisnixObjectClass))
