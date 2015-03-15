/*jshint browser: true, devel: true*/
/*global jQuery, Drupal, generateSelector */

Drupal.versionViewer = Drupal.versionViewer || {};
Drupal.versionViewer.tooltips = Drupal.versionViewer.tooltips || {};

(function ($) {
  'use strict';

  // Each tooltip can belong to at most one entity we keep a reference to each
  // here.
  var tooltips = {};

  // Keep track of hide timer if we need to reset them.
  var hideTooltipTimers = {};

  // Keep track of what positions are occupied.
  var occupiedPositions = {
    top: [],
    left: [],
    right: [],
    bottom: []
  };

  // Keep track of the mouse position for placement of tooltips.
  var mouse = {
    x: 0,
    y: 0
  };

  // Keep track of the mouse position at all times.
  $(document).bind('mousemove.tooltips', function (event) {
    mouse.x = event.pageX;
    mouse.y = event.pageY;
  });

  /**
   * Resets the hidden variables to their initial state.
   */
  function reset() {
    tooltips = {};
    hideReset();
  }

  /**
   * Resets the hidden variables to their initial state.
   */
  function hideReset() {
    hideTooltipTimers = {};
    occupiedPositions = {
      top: [],
      left: [],
      right: [],
      bottom: []
    };
  }

  /**
   * Entity can display a tooltip.
   * 
   * @param {Object} entity An object containing the entity data.
   * @param {String} entity.descriptiveNote The text to display in a tooltip.
   *
   * @return {boolean}
   *   True if the entity can display a tooltip, false otherwise.
   */
  Drupal.versionViewer.tooltips.canDisplayEntity = function (entity) {
    return entity.descriptiveNote !== undefined &&
      entity.descriptiveNote !== null &&
      entity.descriptiveNote.length > 0;
  };

  /**
   * Entity has a tooltip associated with it, although it may not be displayed.
   *
   * @param {String} entID The identifier of the given entity.
   *
   * @return {boolean}
   *   True if the entity has a tooltip object associated with it, false
   *   otherwise.
   */
  Drupal.versionViewer.tooltips.entityHasTooltip = function (entID) {
    return tooltips.hasOwnProperty(entID);
  };

  /**
   * Gets the target element in which to bind the tooltip to.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   *
   * @return {jQuery}
   *   The element to bind the tooltip to.
   */
  function getTooltipTarget(entID, entity) {
    var selector = generateSelector(entID, entity);
    // We filter out the super / sub / line magic elements if they are hidden.
    // We then use only the first matching element, in the case of overlapping
    // entities.
    return $(selector).filter(function() {
      return $(this).css('visibility') !== 'hidden';
    }).first();
  }

  /**
   * Checks if the given entity has claimed a position.
   *
   * @param {String} entID The identifier of the given entity.
   *
   * @return {String|boolean}
   *    The position the entity currently occupies or false if it does not
   *    occupy any position.
   */
  function hasPosition(entID) {
    var occupiedPosition = false;
    $.each(occupiedPositions, function (position, entities) {
      $.each(entities, function (id) {
        if (id === entID) {
          occupiedPosition = position;
          return false;
        }
      });
      return occupiedPosition !== false;
    });
    return occupiedPosition;
  }

  /**
   * Claims the given position so that other tooltips will not to use it.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {String} position The position to claim.
   */
  function claimPosition(entID, position) {
    occupiedPositions[position].push(entID);
  }

  /**
   * Removes the given Entity from it's occupied position.
   *
   * Will remove multiple occurrences although that case should not arise.
   *
   * @param {String} entID The identifier of the given entity.
   */
  function relinquishOccupiedPosition(entID) {
    $.each(occupiedPositions, function (position, entities) {
      occupiedPositions[position] = $.grep(entities, function (id) {
        return id !== entID;
      });
    });
  }

  /**
   * Gets a safe tooltip position for the given entity.
   */
  function getSafeTooltipPosition() {
    var leastOccupiedPosition = 'top';
    var leastEntitiesInPosition = Infinity;
    // Loop through positions looking for a vacant spot, or one with the
    // least entities present.
    $.each(occupiedPositions, function(position, entities) {
      if (entities.length < leastEntitiesInPosition) {
        leastOccupiedPosition = position;
        leastEntitiesInPosition = entities.length;
      }
    });
    return leastOccupiedPosition;
  }

  /**
   * Generates the content for the tooltip from the given entity.
   *
   * Assumes the given entity is valid according to canDisplayEntity().
   *
   * @param {Object} entity An object containing the entity data.
   * @param {String} entity.descriptiveNote The text to display in a tooltip.
   */
  function getTooltipContent(entity) {
    var content = entity.descriptiveNote;
    return '<div class="easyui-panel entity_tooltip">' + content + '</div>';
  }

  /**
   * Event callback that will show a tooltip.
   *
   * @param {object} event
   */
  function showTooltipEvent(event) {
    Drupal.versionViewer.tooltips.showEntityTooltip(event.data.entID);
  }

  /**
   * Event callback that will hide a tooltip.
   *
   * @param {object} event
   */
  function hideTooltipEvent(event) {
    var entID = event.data.entID;
    var options;
    var hasTimer = hideTooltipTimers.hasOwnProperty(entID);
    // Prevent multiple timeout callbacks.
    if (hasTimer) {
      clearTimeout(hideTooltipTimers[entID]);
    }
    if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
        options = tooltips[entID].tooltip('options');
        hideTooltipTimers[entID] = setTimeout((function() {
          function hideTooltipIfApplicable() {
            var ignore = isCursorOverEntity(entID) || isCursorOverTooltip(entID);
            if (!ignore) {
              Drupal.versionViewer.tooltips.hideEntityTooltip(entID);
            }
            else {
              // Set the timeout again so we continue to check if the user has
              // moved the cursor of the entity or the tooltip.
              setTimeout(hideTooltipIfApplicable, options.hideDelay);
            }
          }
          return hideTooltipIfApplicable;
        }()), options.hideDelay);
      }
  }

  /**
   * Checks if the cursor is hovering over the given entity.
   *
   * Overlapping entities can span multiple elements, as such we need to check
   * multiple elements to determine if we should hide them.
   *
   * @param {String} entID The identifier of the given entity.
   *
   * @returns {boolean}
   *   True if the given entity is still hovered over, false otherwise.
   */
  function isCursorOverEntity(entID) {
    var selector = 'span.overlap-spanning-annotation.' + entID;
    return $(selector).filter(function () {
      return $(this).is(':hover');
    }).length !== 0;
  }

  /**
   * Checks if the cursor is hovering over the given entity's tooltip.
   *
   * @param {String} entID The identifier of the given entity.
   *
   * @returns {boolean}
   *   True if the given entity's tooltip is hovered over, false otherwise.
   */
  function isCursorOverTooltip(entID) {
    var tip;
    if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
      tip = tooltips[entID].tooltip('tip');
      if (tip) {
        return tip.is(':hover');
      }
    }
    return false;
  }

  /**
   * Sets custom event handlers on the given entity.
   *
   * This allows us to keep track of whats being displayed as well as enabling
   * overlapping entities to display for more than one element.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   */
  function setCustomEventHandlers(entID, entity) {
    var selector = generateSelector(entID, entity);
    var data = {
      entID: entID,
      entity: entity
    };
    // Remove previous event handlers so we don't compete with the default one.
    // Before doing so we must first show / hide the tooltip so that it's
    // properly initialized. We don't use init with our handlers as they are not
    // yet configured.
    tooltips[entID].tooltip('show');
    tooltips[entID].tooltip('hide');
    $(selector).unbind('mouseenter.tooltip');
    $(selector).unbind('mouseleave.tooltip');
    $(selector).unbind('mousemove.tooltip');
    // Bind out custom handlers, required to deal with overlapping entities, and
    // displaying tooltips in the proper location.
    $(selector).bind('mouseenter', data, showTooltipEvent);
    $(selector).bind('mouseleave', data, hideTooltipEvent);
  }

  /**
   * Removes custom event handlers on the given entity.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   */
  function removeCustomEventHandlers(entID, entity) {
    var selector = generateSelector(entID, entity);
    $(selector).unbind('mouseenter', showTooltipEvent);
    $(selector).unbind('mouseleave', hideTooltipEvent);
  }

  /**
   * Reposition the tooltip so it doesn't overlap others.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {jQuery} tooltip The element the tooltip widget is bound to.
   * @param {function} tooltip.tooltip The tooltip widget.
   */
   function repositionEntityTooltip(entID, tooltip) {
    var deltaX = 0, deltaY = 0;
    var position = hasPosition(entID);
    // Only reposition if the entity has not already been positioned.
    if (position !== false) {
      return;
    }
    position = getSafeTooltipPosition(entID);
    claimPosition(entID, position);
    switch (position) {
      case 'left':
        deltaX = -35;
        break;

      case 'right':
        deltaX = 35;
        break;

      case 'top':
        deltaY = -10;
        break;

      case 'bottom':
        deltaY = 10;
        break;
    }
    tooltip.tooltip({
      position: position,
      deltaX: deltaX,
      deltaY: deltaY
    });
    // We have to unbind the following events every time we change a tooltips
    // options, as the the tooltip plugin will attempt to reset them.
    tooltip.unbind('mouseenter.tooltip');
    tooltip.unbind('mouseleave.tooltip');
    tooltip.unbind('mousemove.tooltip');
    tooltip.tooltip('reposition');
  }

  /**
   * Moves the tooltip based on it's position and the cursor's position.
   *
   * @param {jQuery} tooltip The element the tooltip widget is bound to.
   * @param {function} tooltip.tooltip The tooltip widget.
   */
  function moveTooltipToCursor(tooltip) {
    var tip = tooltip.tooltip('tip');
    var options = tooltip.tooltip('options');
    var x = mouse.x + options.deltaX;
    var y = mouse.y + options.deltaY;
    var anchor = tooltip;
    switch(options.position) {
      case 'right':
        x += anchor.outerWidth() + 24;
        y -= (tip.outerHeight() - anchor.outerHeight()) / 2;
        break;
      case 'left':
        x -= tip.outerWidth() + 24;
        y -= (tip.outerHeight() - anchor.outerHeight()) / 2;
        break;
      case 'top':
        x -= (tip.outerWidth() - anchor.outerWidth()) / 2;
        y -= tip.outerHeight() + 24;
        break;
      case 'bottom':
        x -= (tip.outerWidth() - anchor.outerWidth()) / 2;
        y += anchor.outerHeight() + 24;
        break;
    }
    tip.css({left: x, top: y});
  }

  /**
   * Reset the tooltip content scrollable area to the top.
   *
   * @param {jQuery} tooltip The element the tooltip widget is bound to.
   * @param {function} tooltip.tooltip The tooltip widget.
   */
  function scrollTooltipContentToTop(tooltip) {
    var tip = tooltip.tooltip('tip');
    $('.tooltip-content .easyui-panel', tip).scrollTop(0);
  }

  /**
   * Creates a new tooltip object.
   */
  Drupal.versionViewer.tooltips.createEntityTooltip = function (entID, entity) {
    var target = getTooltipTarget(entID, entity);
    tooltips[entID] = target.tooltip({
      position: 'top',
      content: getTooltipContent(entity),
      showDelay: 200,
      hideDelay: 500,
      onShow: function () {
        var tooltip = $(this);
        scrollTooltipContentToTop(tooltip);
        moveTooltipToCursor(tooltip);
      },
      onDestroy: function () {
        removeCustomEventHandlers(entID,  entity);
      }
    });
    // We set our own custom handlers so we can keep track of whats displaying
    // and so we can show tooltips over top of more than one element
    // (for overlapping entities).
    setCustomEventHandlers(entID, entity);
  };

  /**
   * Shows the given tooltip.
   *
   * @param {String} entID The identifier of the given entity.
   */
  Drupal.versionViewer.tooltips.showEntityTooltip = function (entID) {
    var tooltip, tip, initialized, hidden;
    if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
      tooltip = tooltips[entID];
      tip = tooltip.tooltip('tip');
      initialized = tip !== undefined;
      hidden = !initialized || tooltip.tooltip('tip').is(':hidden');
      // Only display if it's not hidden.
      if (hidden) {
        // Reposition before showing to avoid glitches as the 'onShow' event
        // isn't called until after the tooltip has already been displayed.
        repositionEntityTooltip(entID, tooltip);
        tooltip.tooltip('show');
      }
    }
  };

  /**
   * Hides the given tooltip.
   *
   * @param {String} entID The identifier of the given entity.
   */
  Drupal.versionViewer.tooltips.hideEntityTooltip = function (entID) {
    var tooltip, tip, initialized, visible;
    if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
      tooltip = tooltips[entID];
      try {
        tip = tooltip.tooltip('tip');
      } catch(e) {
        (console.error || console.log).call(console, e.stack || e);
      }
      initialized = tip !== undefined;
      visible = !initialized || tooltip.tooltip('tip').is(':visible');
      if (visible) {
        tooltip.tooltip('hide');
        relinquishOccupiedPosition(entID);
      }
    }
  };

  /**
   * Destroys all the tooltips.
   */
  Drupal.versionViewer.tooltips.destroyEntityTooltip = function (entID) {
    if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
      relinquishOccupiedPosition(entID);
      tooltips[entID].tooltip('destroy');
      delete tooltips[entID];
    }
  };

  /**
   * Shows all of the tooltips.
   */
  Drupal.versionViewer.tooltips.showTooltips = function () {
    $.each(tooltips, function (entID) {
      if (Drupal.versionViewer.tooltips.entityHasTooltip(entID)) {
        Drupal.versionViewer.tooltips.showEntityTooltip(entID);
      }
    });
  };

  /**
   * Shows the given tooltip.
   */
  Drupal.versionViewer.tooltips.hideTooltips = function () {
    $.each(tooltips, function (entID) {
      Drupal.versionViewer.tooltips.hideEntityTooltip(entID);
    });
    // Reset all just to be safe.
    hideReset();
  };

  /**
   * Destroys all the tooltips.
   */
  Drupal.versionViewer.tooltips.destroyTooltips = function () {
    $.each(tooltips, function (entID) {
      Drupal.versionViewer.tooltips.destroyEntityTooltip(entID);
    });
    // Reset all just to be safe.
    reset();
  };

}(jQuery));
