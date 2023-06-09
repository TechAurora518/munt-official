// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

/**
 * Interface to handle result of RPC commands
 * Calls either onSuccess or onError depending on whether command suceedes or fails
 */
public abstract class IRpcListener {
    /**
     * Returns a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    public abstract void onFilteredCommand(String filteredCommand);

    /**
     * Returns the result and a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    public abstract void onSuccess(String filteredCommand, String result);

    /**
     * Returns an error message which might be a plain string or JSON depending on the type of error
     * Also returns a filtered version of the command with sensitive information like passwords removed
     * Any kind of 'command history' functionality should store this filtered command and not the original command
     */
    public abstract void onError(String filteredCommand, String errorMessage);
}
